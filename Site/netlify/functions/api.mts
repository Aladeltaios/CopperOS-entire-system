import type { Config, Context } from "@netlify/functions";
import { randomBytes } from "node:crypto";
import { and, desc, eq, gt } from "drizzle-orm";
import { db } from "../../db/index.js";
import { bans, posts, users } from "../../db/schema.js";

// Naive keyword moderation, carried over from the original server.
const BLACKLIST = ["viagra", "malware", "phish", "spam"];
const BAN_MS = 15 * 60 * 1000;

function moderate(content: string): boolean {
  const s = (content || "").toLowerCase();
  return BLACKLIST.some((w) => s.includes(w));
}

function genToken(): string {
  return randomBytes(16).toString("hex");
}

function json(data: unknown, status = 200): Response {
  return Response.json(data, { status });
}

function bearer(req: Request): string | null {
  const auth = req.headers.get("authorization");
  if (!auth) return null;
  const parts = auth.split(" ");
  return parts[1] || null;
}

async function currentUser(req: Request) {
  const token = bearer(req);
  if (!token) return null;
  const [u] = await db.select().from(users).where(eq(users.token, token));
  return u || null;
}

function isAdmin(req: Request): boolean {
  const code = Netlify.env.get("ADMIN_CODE");
  // Admin endpoints stay disabled until an ADMIN_CODE is configured.
  if (!code) return false;
  return req.headers.get("x-admin-code") === code;
}

// Public shape of a post — never exposes author tokens.
function publicPost(p: typeof posts.$inferSelect, authorName: string, country: string) {
  return {
    id: p.id,
    title: p.title,
    content: p.content,
    authorId: p.authorId,
    authorName,
    country,
    flagged: p.flagged,
    ts: p.createdAt ? new Date(p.createdAt).getTime() : Date.now(),
  };
}

export default async (req: Request, context: Context): Promise<Response> => {
  const url = new URL(req.url);
  const path = url.pathname.replace(/\/+$/, "");
  const method = req.method.toUpperCase();

  // POST /api/register
  if (path === "/api/register" && method === "POST") {
    const { username, country } = (await req.json().catch(() => ({}))) as {
      username?: string;
      country?: string;
    };
    if (!username || !country) return json({ error: "username and country required" }, 400);
    const existing = await db.select().from(users);
    if (existing.find((u) => u.username.toLowerCase() === username.toLowerCase())) {
      return json({ error: "username taken" }, 400);
    }
    const token = genToken();
    const [u] = await db.insert(users).values({ username, country, token }).returning();
    return json({ ok: true, token });
  }

  // GET/PUT /api/me
  if (path === "/api/me") {
    const u = await currentUser(req);
    if (!u) return json({ error: "no auth" }, 401);
    if (method === "GET") {
      return json({ id: u.id, username: u.username, country: u.country });
    }
    if (method === "PUT") {
      const body = (await req.json().catch(() => ({}))) as { username?: string };
      if (body.username) {
        await db.update(users).set({ username: body.username }).where(eq(users.id, u.id));
      }
      return json({ ok: true });
    }
  }

  // GET/POST /api/posts
  if (path === "/api/posts") {
    if (method === "GET") {
      const rows = await db
        .select({ post: posts, authorName: users.username, country: users.country })
        .from(posts)
        .innerJoin(users, eq(posts.authorId, users.id))
        .orderBy(desc(posts.createdAt));
      return json({ posts: rows.map((r) => publicPost(r.post, r.authorName, r.country)) });
    }
    if (method === "POST") {
      const u = await currentUser(req);
      if (!u) return json({ error: "no auth" }, 401);

      const activeBan = await db
        .select()
        .from(bans)
        .where(and(eq(bans.userId, u.id), gt(bans.expires, new Date())));
      if (activeBan.length) {
        return json({ error: "banned", until: new Date(activeBan[0].expires).getTime() }, 403);
      }

      const { title, content } = (await req.json().catch(() => ({}))) as {
        title?: string;
        content?: string;
      };
      const flagged = moderate(`${title || ""} ${content || ""}`);
      await db
        .insert(posts)
        .values({ title: title || "", content: content || "", authorId: u.id, flagged });

      if (flagged) {
        // Auto-ban for 15 minutes and record an alert in the function logs.
        await db.insert(bans).values({ userId: u.id, expires: new Date(Date.now() + BAN_MS) });
        console.warn(
          `CopperOS discussion flagged: user=${u.username} (id=${u.id}, country=${u.country}) auto-banned 15m`,
        );
      }
      return json({ ok: true, flagged });
    }
  }

  // PUT/DELETE /api/posts/:id
  const postMatch = path.match(/^\/api\/posts\/(\d+)$/);
  if (postMatch) {
    const id = Number(postMatch[1]);
    const u = await currentUser(req);
    if (!u) return json({ error: "no auth" }, 401);
    const [p] = await db.select().from(posts).where(eq(posts.id, id));
    if (!p) return json({ error: "not found" }, 404);

    if (method === "PUT") {
      if (p.authorId !== u.id) return json({ error: "forbidden" }, 403);
      const body = (await req.json().catch(() => ({}))) as { title?: string; content?: string };
      const update: Record<string, string> = {};
      if (body.title !== undefined) update.title = body.title;
      if (body.content !== undefined) update.content = body.content;
      if (Object.keys(update).length) await db.update(posts).set(update).where(eq(posts.id, id));
      return json({ ok: true });
    }
    if (method === "DELETE") {
      if (p.authorId !== u.id && !isAdmin(req)) return json({ error: "forbidden" }, 403);
      await db.delete(posts).where(eq(posts.id, id));
      return json({ ok: true });
    }
  }

  // ---- Admin endpoints (require x-admin-code matching ADMIN_CODE env var) ----
  if (path === "/api/admin/users" && method === "GET") {
    if (!isAdmin(req)) return json({ error: "forbidden" }, 403);
    const rows = await db
      .select({ id: users.id, username: users.username, country: users.country, createdAt: users.createdAt })
      .from(users);
    return json({ users: rows });
  }

  if (path === "/api/admin/posts" && method === "GET") {
    if (!isAdmin(req)) return json({ error: "forbidden" }, 403);
    const rows = await db
      .select({ post: posts, authorName: users.username, country: users.country })
      .from(posts)
      .innerJoin(users, eq(posts.authorId, users.id))
      .orderBy(desc(posts.createdAt));
    return json({ posts: rows.map((r) => publicPost(r.post, r.authorName, r.country)) });
  }

  if (path === "/api/admin/ban" && method === "POST") {
    if (!isAdmin(req)) return json({ error: "forbidden" }, 403);
    const { userId, minutes } = (await req.json().catch(() => ({}))) as {
      userId?: number | string;
      minutes?: number;
    };
    if (!userId) return json({ error: "userId required" }, 400);
    await db
      .insert(bans)
      .values({ userId: Number(userId), expires: new Date(Date.now() + (minutes || 15) * 60 * 1000) });
    return json({ ok: true });
  }

  const adminPostMatch = path.match(/^\/api\/admin\/posts\/(\d+)$/);
  if (adminPostMatch && method === "DELETE") {
    if (!isAdmin(req)) return json({ error: "forbidden" }, 403);
    await db.delete(posts).where(eq(posts.id, Number(adminPostMatch[1])));
    return json({ ok: true });
  }

  return json({ error: "not found" }, 404);
};

export const config: Config = {
  path: "/api/*",
};
