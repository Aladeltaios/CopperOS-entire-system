// Client that talks to the server API for real discussions.
(function(){
  const TOKEN_KEY = 'copperos_token';
  const API_ROOT = '/api';

  function token(){ return localStorage.getItem(TOKEN_KEY); }
  function setToken(t){ if(t) localStorage.setItem(TOKEN_KEY, t); else localStorage.removeItem(TOKEN_KEY); renderUserArea(); renderUserBanner(); }

  async function api(path, opts={}){
    opts.headers = opts.headers || {};
    if(token()) opts.headers['Authorization'] = 'Bearer ' + token();
    const res = await fetch(API_ROOT + path, Object.assign({credentials:'same-origin'}, opts));
    if(res.status===401){ setToken(null); throw new Error('Unauthorized'); }
    return res.json();
  }

  function escapeHtml(s){ return String(s||'').replace(/[&<>"']/g, function(c){return{'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;',"'":'&#39;'}[c];}); }

  async function renderUserBanner(){ const el = document.getElementById('copperos-user-banner'); if(!el) return;
    try{
      const me = await api('/me');
      el.innerHTML = 'Signed in as <strong>'+escapeHtml(me.username)+'</strong> — <a href="/discussions/admin.html">Admin</a>';
    }catch(e){ el.innerHTML = '<a href="/discussions/index.html">Sign in / Create username</a>'; }
  }

  async function renderUserArea(){ const area = document.getElementById('userArea'); if(!area) return; area.innerHTML='';
    try{
      const me = await api('/me');
      const span = document.createElement('div'); span.innerHTML = 'Signed in as <strong>' + escapeHtml(me.username) + '</strong> ('+escapeHtml(me.country)+')';
      const btn = document.createElement('button'); btn.textContent='Change username'; btn.className='btn small';
      btn.addEventListener('click', async ()=>{
        const n = prompt('Enter new username', me.username);
        if(n && n.trim()){ await api('/me', { method:'PUT', headers:{'Content-Type':'application/json'}, body: JSON.stringify({ username: n.trim() }) }); renderUserArea(); renderUserBanner(); renderPosts(); }
      });
      const out = document.createElement('button'); out.textContent='Logout'; out.className='btn small secondary';
      out.addEventListener('click', ()=>{ if(confirm('Log out?')){ setToken(null); renderUserArea(); renderPosts(); renderUserBanner(); } });
      area.appendChild(span); area.appendChild(btn); area.appendChild(out);
    }catch(e){
      // show register form
      const form = document.createElement('div');
      const input = document.createElement('input'); input.placeholder='Choose a username';
      const country = document.createElement('select'); country.innerHTML = '<option value="">Select country</option>' + countryOptions();
      const btn = document.createElement('button'); btn.textContent='Create account'; btn.className='btn small';
      btn.addEventListener('click', async ()=>{
        const v = input.value && input.value.trim(); const c = country.value; if(!v || !c) return alert('Enter username and choose country');
        try{
          const res = await fetch('/api/register',{ method:'POST', headers:{'Content-Type':'application/json'}, body: JSON.stringify({ username: v, country: c }) });
          const body = await res.json(); if(res.ok && body.token){ setToken(body.token); renderUserArea(); renderPosts(); renderUserBanner(); }
          else alert(body.error || 'Registration failed');
        }catch(err){ alert('Registration failed'); }
      });
      form.appendChild(input); form.appendChild(country); form.appendChild(btn);
      area.appendChild(form);
    }
  }

  function countryOptions(){ // small list; extend as desired
    const list = ['United States','United Kingdom','Spain','Germany','France','India','China','Brazil','Mexico','Other'];
    return list.map(c=>'\n<option value="'+c+'">'+c+'</option>').join('');
  }

  async function renderPosts(){ const container = document.getElementById('posts'); if(!container) return; container.innerHTML='Loading...';
    try{
      const data = await api('/posts'); container.innerHTML=''; if(!data.posts || data.posts.length===0){ container.innerHTML = '<p class="muted">No discussions yet. Start one!</p>'; return; }
      const me = await safeApi('/me'); const myId = me && me.id;
      data.posts.forEach(post=>{
        const el = document.createElement('article'); el.className='post';
        const meta = document.createElement('div'); meta.className='meta';
        meta.innerHTML = (post.title? escapeHtml(post.title) + ' — ' : '') + 'by ' + escapeHtml(post.authorName||'Anonymous') + ' ('+escapeHtml(post.country||'')+') • ' + new Date(post.ts).toLocaleString();
        const body = document.createElement('div'); body.className='body'; body.innerHTML = '<div>' + escapeHtml(post.content).replace(/\n/g,'<br>') + '</div>';
        const controls = document.createElement('div'); controls.className='controls';
        if(myId && post.authorId === myId){
          const edit = document.createElement('button'); edit.textContent='Edit'; edit.className='btn small';
          edit.addEventListener('click', ()=> editPostUI(post));
          const del = document.createElement('button'); del.textContent='Delete'; del.className='btn small secondary';
          del.addEventListener('click', async ()=>{ if(!confirm('Delete this post?')) return; await api('/posts/'+post.id,{ method:'DELETE' }); renderPosts(); });
          controls.appendChild(edit); controls.appendChild(del);
        }
        el.appendChild(meta); el.appendChild(body); el.appendChild(controls);
        container.appendChild(el);
      });
    }catch(e){ container.innerHTML = '<p class="muted">Unable to load posts.</p>'; }
  }

  async function safeApi(path, opts){ try{ return await api(path, opts); }catch(e){ return {}; } }

  function editPostUI(post){ const newTitle = prompt('Edit title', post.title); if(newTitle===null) return; const newContent = prompt('Edit content', post.content); if(newContent===null) return;
    api('/posts/'+post.id, { method:'PUT', headers:{'Content-Type':'application/json'}, body: JSON.stringify({ title: newTitle, content: newContent }) }).then(()=>renderPosts()).catch(()=>alert('Failed'));
  }

  function wireNewPost(){ const pub = document.getElementById('publishBtn'); const clear = document.getElementById('clearBtn'); if(pub){ pub.addEventListener('click', async ()=>{
      const title = document.getElementById('postTitle').value.trim(); const content = document.getElementById('postContent').value.trim(); if(!content && !title) return alert('Write a title or content');
      try{
        const res = await api('/posts',{ method:'POST', headers:{'Content-Type':'application/json'}, body: JSON.stringify({ title, content }) });
        const body = await res; if(body && body.ok){ document.getElementById('postTitle').value=''; document.getElementById('postContent').value=''; renderPosts(); }
        else alert(body.error || 'Publish failed');
      }catch(err){ alert(err.message || 'Publish failed'); }
    }); }
    if(clear){ clear.addEventListener('click', ()=>{ document.getElementById('postTitle').value=''; document.getElementById('postContent').value=''; }); }
  }

  document.addEventListener('DOMContentLoaded', ()=>{ renderUserArea(); renderPosts(); renderUserBanner(); wireNewPost(); });

  // expose for debugging
  window.CopperOSDiscussions = { setToken, token };

})();
