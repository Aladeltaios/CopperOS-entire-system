const express = require('express');
const fs = require('fs').promises;
const path = require('path');
const crypto = require('crypto');
const nodemailer = require('nodemailer');

const DATA_DIR = path.join(__dirname, 'data');
const USERS_FILE = path.join(DATA_DIR, 'users.json');
const POSTS_FILE = path.join(DATA_DIR, 'posts.json');
const BANS_FILE = path.join(DATA_DIR, 'bans.json');
const ALERT_EMAIL = 'alex1yegh@gmail.com'; // recipient for alerts
const ADMIN_CODE = 'qwerty12345';

async function ensure() {
  await fs.mkdir(DATA_DIR, { recursive: true });
  for (const f of [USERS_FILE, POSTS_FILE, BANS_FILE]) {
    try { await fs.access(f); } catch (e) { await fs.writeFile(f, '[]'); }
  }
}

async function readJSON(file){ try{ const s = await fs.readFile(file,'utf8'); return JSON.parse(s||'[]'); }catch(e){ return []; } }
async function writeJSON(file, obj){ await fs.writeFile(file, JSON.stringify(obj, null, 2)); }

function genToken(){ return crypto.randomBytes(16).toString('hex'); }

// Simple transporter using env vars; if not configured, fallback to console logging
function getTransport(){
  const host = process.env.SMTP_HOST, port = process.env.SMTP_PORT, user = process.env.SMTP_USER, pass = process.env.SMTP_PASS;
  if(host && user){ return nodemailer.createTransport({ host, port: Number(port)||587, secure:false, auth:{ user, pass } }); }
  return null;
}

async function sendAlert(subject, text){
  const transporter = getTransport();
  if(transporter){
    await transporter.sendMail({ from: process.env.SMTP_FROM || 'alerts@example.com', to: ALERT_EMAIL, subject, text });
    console.log('Alert email sent to', ALERT_EMAIL);
  } else {
    // fallback: write to alerts.log
    await fs.appendFile(path.join(DATA_DIR,'alerts.log'), new Date().toISOString() + ' - ' + subject + '\n' + text + '\n\n');
    console.log('No SMTP configured — logged alert to data/alerts.log');
  }
}

// Moderation: naive blacklisted keywords
const BLACKLIST = ['viagra','malware','phish','spam'];
function moderate(content){
  const s = (content||'').toLowerCase();
  return BLACKLIST.some(w=> s.includes(w));
}

async function run(){
  await ensure();
  const app = express();
  app.use(express.json());
  app.use(express.static(__dirname));

  app.post('/api/register', async (req,res)=>{
    const { username, country } = req.body || {};
    if(!username || !country) return res.status(400).json({ error: 'username and country required' });
    const users = await readJSON(USERS_FILE);
    if(users.find(u=>u.username.toLowerCase()===username.toLowerCase())) return res.status(400).json({ error: 'username taken' });
    const token = genToken();
    const user = { id: Date.now()+Math.floor(Math.random()*1000), username, country, token, created: Date.now() };
    users.push(user); await writeJSON(USERS_FILE, users);
    res.json({ ok:true, token });
  });

  // Get current user
  app.get('/api/me', async (req,res)=>{
    const auth = req.headers['authorization'];
    if(!auth) return res.status(401).json({ error: 'no auth' });
    const token = auth.split(' ')[1];
    const users = await readJSON(USERS_FILE); const u = users.find(x=>x.token===token);
    if(!u) return res.status(401).json({ error: 'invalid' });
    res.json({ id: u.id, username: u.username, country: u.country });
  });

  app.put('/api/me', async (req,res)=>{
    const auth = req.headers['authorization']; if(!auth) return res.status(401).json({ error: 'no auth' });
    const token = auth.split(' ')[1]; const users = await readJSON(USERS_FILE); const u = users.find(x=>x.token===token);
    if(!u) return res.status(401).json({ error: 'invalid' });
    if(req.body.username) u.username = req.body.username;
    await writeJSON(USERS_FILE, users); res.json({ ok:true });
  });

  // Posts
  app.get('/api/posts', async (req,res)=>{
    const posts = await readJSON(POSTS_FILE);
    // return ordered descending
    posts.sort((a,b)=>b.ts - a.ts);
    res.json({ posts });
  });

  app.post('/api/posts', async (req,res)=>{
    const auth = req.headers['authorization']; if(!auth) return res.status(401).json({ error: 'no auth' });
    const token = auth.split(' ')[1]; const users = await readJSON(USERS_FILE); const u = users.find(x=>x.token===token);
    if(!u) return res.status(401).json({ error: 'invalid' });

    // check ban
    const bans = await readJSON(BANS_FILE); const ban = bans.find(b=>b.userId===u.id && b.expires > Date.now());
    if(ban) return res.status(403).json({ error: 'banned', until: ban.expires });

    const { title, content } = req.body || {};
    const flagged = moderate((title||'') + ' ' + (content||''));
    const posts = await readJSON(POSTS_FILE);
    const post = { id: Date.now()+Math.floor(Math.random()*1000), title: title||'', content: content||'', authorId: u.id, authorName: u.username, country: u.country, ts: Date.now(), authorToken: token };
    posts.push(post); await writeJSON(POSTS_FILE, posts);

    if(flagged){
      // alert and ban 15 minutes
      const subject = 'CopperOS discussion flagged';
      const text = `Post by ${u.username} (id=${u.id}, country=${u.country}) was flagged and will be auto-banned for 15 minutes.\n\nTitle: ${title}\n\nContent:\n${content}`;
      await sendAlert(subject, text).catch(()=>{});
      const bans = await readJSON(BANS_FILE); bans.push({ userId: u.id, expires: Date.now() + 15*60*1000 }); await writeJSON(BANS_FILE, bans);
    }

    res.json({ ok:true, flagged: !!flagged });
  });

  app.put('/api/posts/:id', async (req,res)=>{
    const auth = req.headers['authorization']; if(!auth) return res.status(401).json({ error: 'no auth' });
    const token = auth.split(' ')[1]; const users = await readJSON(USERS_FILE); const u = users.find(x=>x.token===token);
    if(!u) return res.status(401).json({ error: 'invalid' });
    const posts = await readJSON(POSTS_FILE); const p = posts.find(x=>String(x.id)===String(req.params.id)); if(!p) return res.status(404).json({ error: 'not found' });
    if(p.authorId !== u.id) return res.status(403).json({ error: 'forbidden' });
    if(req.body.title !== undefined) p.title = req.body.title; if(req.body.content !== undefined) p.content = req.body.content;
    await writeJSON(POSTS_FILE, posts); res.json({ ok:true });
  });

  app.delete('/api/posts/:id', async (req,res)=>{
    const auth = req.headers['authorization']; if(!auth) return res.status(401).json({ error: 'no auth' });
    const token = auth.split(' ')[1]; const users = await readJSON(USERS_FILE); const u = users.find(x=>x.token===token);
    if(!u) return res.status(401).json({ error: 'invalid' });
    let posts = await readJSON(POSTS_FILE); const p = posts.find(x=>String(x.id)===String(req.params.id)); if(!p) return res.status(404).json({ error: 'not found' });
    // allow if owner or admin code header
    const adminHeader = req.headers['x-admin-code'];
    if(p.authorId !== u.id && adminHeader !== ADMIN_CODE) return res.status(403).json({ error: 'forbidden' });
    posts = posts.filter(x=>String(x.id)!==String(req.params.id)); await writeJSON(POSTS_FILE, posts); res.json({ ok:true });
  });

  // Admin APIs — require header x-admin-code
  function isAdmin(req){ return req.headers['x-admin-code'] === ADMIN_CODE; }

  app.get('/api/admin/users', async (req,res)=>{
    if(!isAdmin(req)) return res.status(403).json({ error: 'forbidden' });
    const users = await readJSON(USERS_FILE); res.json({ users });
  });

  app.get('/api/admin/posts', async (req,res)=>{
    if(!isAdmin(req)) return res.status(403).json({ error: 'forbidden' });
    const posts = await readJSON(POSTS_FILE); res.json({ posts });
  });

  app.post('/api/admin/ban', async (req,res)=>{
    if(!isAdmin(req)) return res.status(403).json({ error: 'forbidden' });
    const { userId, minutes } = req.body || {};
    if(!userId) return res.status(400).json({ error: 'userId required' });
    const bans = await readJSON(BANS_FILE); bans.push({ userId, expires: Date.now() + (minutes||15)*60*1000 }); await writeJSON(BANS_FILE, bans); res.json({ ok:true });
  });

  app.delete('/api/admin/posts/:id', async (req,res)=>{
    if(!isAdmin(req)) return res.status(403).json({ error: 'forbidden' });
    let posts = await readJSON(POSTS_FILE); posts = posts.filter(x=>String(x.id)!==String(req.params.id)); await writeJSON(POSTS_FILE, posts); res.json({ ok:true });
  });

  // Admin page served as static file; API checks admin header

  const PORT = process.env.PORT || 3000;
  app.listen(PORT, ()=> console.log('Server listening on', PORT));
}

run().catch(err=>{ console.error(err); process.exit(1); });
