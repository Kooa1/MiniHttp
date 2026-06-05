document.addEventListener('DOMContentLoaded', async () => {
  const container = document.getElementById('routes-container');
  const desc = document.querySelector('.section-desc');

  try {
    const resp = await fetch('/api/routes');
    if (!resp.ok) throw new Error('HTTP ' + resp.status);
    const data = await resp.json();

    desc.textContent = 'Fetched from /api/routes — ' + data.routes.length + ' routes registered.';

    if (!data.routes.length) {
      container.innerHTML = '<p class="loading">No routes registered.</p>';
      return;
    }

    const table = document.createElement('table');
    table.className = 'routes-table';

    const thead = document.createElement('thead');
    thead.innerHTML = '<tr><th>Method</th><th>Path</th><th>Handler</th></tr>';
    table.appendChild(thead);

    const tbody = document.createElement('tbody');
    for (const route of data.routes) {
      const tr = document.createElement('tr');
      tr.innerHTML = '<td><span class="method-get">' + route.method + '</span></td>'
                   + '<td>' + route.path + '</td>'
                   + '<td>' + route.handler + '</td>';
      tbody.appendChild(tr);
    }
    table.appendChild(tbody);
    container.innerHTML = '';
    container.appendChild(table);

  } catch (err) {
    desc.textContent = 'Failed to load routes.';
    container.innerHTML = '<p class="loading" style="color:#e53e3e;">Error: ' + err.message + '</p>';
  }
});
