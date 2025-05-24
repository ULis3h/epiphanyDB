class StorageMonitor {
    constructor() {
        this.currentView = 'dashboard';
        this.cache = new CacheMonitor();
        this.pages = new PageMonitor();
        this.btree = new BTreeVisualizer();
        this.logs = new LogMonitor();
        
        this.initEventListeners();
        this.loadView('dashboard');
    }

    initEventListeners() {
        document.querySelectorAll('.nav-item').forEach(item => {
            item.addEventListener('click', () => {
                document.querySelector('.nav-item.active').classList.remove('active');
                item.classList.add('active');
                this.loadView(item.dataset.view);
            });
        });
    }

    async loadView(view) {
        const mainContent = document.getElementById('mainContent');
        mainContent.innerHTML = ''; // Clear current content

        switch(view) {
            case 'dashboard':
                await this.loadDashboard();
                break;
            case 'cache':
                await this.cache.load();
                break;
            case 'pages':
                await this.pages.load();
                break;
            case 'btree':
                await this.btree.load();
                break;
            case 'logs':
                await this.logs.load();
                break;
        }
    }

    async loadDashboard() {
        const mainContent = document.getElementById('mainContent');
        
        // Add statistics cards
        const statsHtml = `
            <div class="panel">
                <h2>System Overview</h2>
                <div class="stats-grid">
                    <div class="stat-card">
                        <h3>Cache Hit Rate</h3>
                        <p id="cacheHitRate">85%</p>
                    </div>
                    <div class="stat-card">
                        <h3>Active Pages</h3>
                        <p id="activePages">1,234</p>
                    </div>
                    <div class="stat-card">
                        <h3>Memory Usage</h3>
                        <p id="memoryUsage">456 MB</p>
                    </div>
                    <div class="stat-card">
                        <h3>Total Operations</h3>
                        <p id="totalOps">789K</p>
                    </div>
                </div>
            </div>
            <div class="panel">
                <h2>Performance Metrics</h2>
                <canvas id="performanceChart"></canvas>
            </div>
        `;
        
        mainContent.innerHTML = statsHtml;
        
        // Initialize performance chart
        this.initPerformanceChart();
    }

    initPerformanceChart() {
        const ctx = document.getElementById('performanceChart').getContext('2d');
        new Chart(ctx, {
            type: 'line',
            data: {
                labels: Array.from({length: 24}, (_, i) => `${i}:00`),
                datasets: [{
                    label: 'Operations per Second',
                    data: Array.from({length: 24}, () => Math.random() * 1000),
                    borderColor: '#3498db',
                    tension: 0.4
                }]
            },
            options: {
                responsive: true,
                maintainAspectRatio: false,
                scales: {
                    y: {
                        beginAtZero: true
                    }
                }
            }
        });
    }
}

class CacheMonitor {
    async load() {
        const mainContent = document.getElementById('mainContent');
        
        const html = `
            <div class="panel">
                <h2>Cache Information</h2>
                <input type="text" class="search-box" placeholder="Search cache entries...">
                <div class="table-container">
                    <table>
                        <thead>
                            <tr>
                                <th>Page ID</th>
                                <th>Status</th>
                                <th>Last Access</th>
                                <th>Hit Count</th>
                                <th>Dirty</th>
                                <th>Actions</th>
                            </tr>
                        </thead>
                        <tbody id="cacheTableBody">
                            <!-- Cache entries will be loaded here -->
                        </tbody>
                    </table>
                </div>
            </div>
        `;
        
        mainContent.innerHTML = html;
        await this.loadCacheData();
    }

    async loadCacheData() {
        // Simulate loading cache data from backend
        const cacheData = await this.fetchCacheData();
        const tbody = document.getElementById('cacheTableBody');
        
        tbody.innerHTML = cacheData.map(entry => `
            <tr>
                <td>${entry.pageId}</td>
                <td>${entry.status}</td>
                <td>${entry.lastAccess}</td>
                <td>${entry.hitCount}</td>
                <td>${entry.dirty ? 'Yes' : 'No'}</td>
                <td>
                    <button onclick="viewPage(${entry.pageId})">View</button>
                    <button onclick="flushPage(${entry.pageId})">Flush</button>
                </td>
            </tr>
        `).join('');
    }

    async fetchCacheData() {
        // Simulate API call
        return [
            {pageId: 1, status: 'Active', lastAccess: '2024-01-01 12:00:00', hitCount: 150, dirty: true},
            {pageId: 2, status: 'Inactive', lastAccess: '2024-01-01 11:55:00', hitCount: 75, dirty: false},
            // ... more entries
        ];
    }
}

class PageMonitor {
    async load() {
        const mainContent = document.getElementById('mainContent');
        
        const html = `
            <div class="panel">
                <h2>Page Status</h2>
                <div class="stats-grid">
                    <div class="stat-card">
                        <h3>Total Pages</h3>
                        <p id="totalPages">10,000</p>
                    </div>
                    <div class="stat-card">
                        <h3>Free Pages</h3>
                        <p id="freePages">2,500</p>
                    </div>
                </div>
                <div class="page-content" id="pageDetails">
                    Select a page to view details...
                </div>
            </div>
        `;
        
        mainContent.innerHTML = html;
    }
}

class BTreeVisualizer {
    async load() {
        const mainContent = document.getElementById('mainContent');
        
        const html = `
            <div class="panel">
                <h2>B+ Tree Visualization</h2>
                <div class="tree-container" id="treeVisualization"></div>
            </div>
        `;
        
        mainContent.innerHTML = html;
        await this.visualizeTree();
    }

    async visualizeTree() {
        const container = document.getElementById('treeVisualization');
        const data = await this.fetchTreeData();
        
        const nodes = new vis.DataSet(data.nodes);
        const edges = new vis.DataSet(data.edges);
        
        const options = {
            layout: {
                hierarchical: {
                    direction: 'UD',
                    sortMethod: 'directed',
                    nodeSpacing: 150,
                    levelSeparation: 150
                }
            },
            nodes: {
                shape: 'box',
                margin: 10,
                font: {
                    size: 14
                }
            },
            edges: {
                arrows: 'to'
            }
        };
        
        new vis.Network(container, {nodes, edges}, options);
    }

    async fetchTreeData() {
        // Simulate API call
        return {
            nodes: [
                {id: 1, label: 'Root [10, 20, 30]'},
                {id: 2, label: '[5, 8]'},
                {id: 3, label: '[15, 18]'},
                {id: 4, label: '[25, 28]'},
                {id: 5, label: '[35, 38]'}
            ],
            edges: [
                {from: 1, to: 2},
                {from: 1, to: 3},
                {from: 1, to: 4},
                {from: 1, to: 5}
            ]
        };
    }
}

class LogMonitor {
    async load() {
        const mainContent = document.getElementById('mainContent');
        
        const html = `
            <div class="panel">
                <h2>System Logs</h2>
                <select id="logLevel">
                    <option value="ALL">All Levels</option>
                    <option value="INFO">Info</option>
                    <option value="WARNING">Warning</option>
                    <option value="ERROR">Error</option>
                </select>
                <div class="table-container">
                    <table>
                        <thead>
                            <tr>
                                <th>Timestamp</th>
                                <th>Level</th>
                                <th>Message</th>
                            </tr>
                        </thead>
                        <tbody id="logTableBody">
                            <!-- Log entries will be loaded here -->
                        </tbody>
                    </table>
                </div>
            </div>
        `;
        
        mainContent.innerHTML = html;
        await this.loadLogs();
    }

    async loadLogs() {
        const logs = await this.fetchLogs();
        const tbody = document.getElementById('logTableBody');
        
        tbody.innerHTML = logs.map(log => `
            <tr class="log-${log.level.toLowerCase()}">
                <td>${log.timestamp}</td>
                <td>${log.level}</td>
                <td>${log.message}</td>
            </tr>
        `).join('');
    }

    async fetchLogs() {
        // Simulate API call
        return [
            {timestamp: '2024-01-01 12:00:00', level: 'INFO', message: 'Cache flush completed'},
            {timestamp: '2024-01-01 11:59:00', level: 'WARNING', message: 'High memory usage detected'},
            // ... more logs
        ];
    }
}

// Initialize the application
const app = new StorageMonitor();