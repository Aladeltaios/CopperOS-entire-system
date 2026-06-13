import { pgTable, serial, text, integer, boolean, timestamp } from "drizzle-orm/pg-core";

export const users = pgTable("users", {
  id: serial().primaryKey(),
  username: text().notNull().unique(),
  country: text().notNull(),
  token: text().notNull().unique(),
  createdAt: timestamp("created_at").defaultNow(),
});

export const posts = pgTable("posts", {
  id: serial().primaryKey(),
  title: text().notNull().default(""),
  content: text().notNull().default(""),
  authorId: integer("author_id")
    .notNull()
    .references(() => users.id),
  flagged: boolean().notNull().default(false),
  createdAt: timestamp("created_at").defaultNow(),
});

export const bans = pgTable("bans", {
  id: serial().primaryKey(),
  userId: integer("user_id")
    .notNull()
    .references(() => users.id),
  expires: timestamp().notNull(),
  createdAt: timestamp("created_at").defaultNow(),
});
