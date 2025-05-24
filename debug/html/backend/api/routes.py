from fastapi import FastAPI, WebSocket
from fastapi.responses import HTMLResponse
import json
import asyncio

app = FastAPI()

class MonitoringSystem:
    def __init__(self):
        self.clients = set()
    
    async def register(self, websocket: WebSocket):
        await websocket.accept()
        self.clients.add(websocket)
    
    async def unregister(self, websocket: WebSocket):
        self.clients.remove(websocket)
    
    async def broadcast(self, message: dict):
        for client in self.clients:
            try:
                await client.send_json(message)
            except:
                await self.unregister(client)

monitor = MonitoringSystem()

@app.websocket("/monitor")
async def websocket_endpoint(websocket: WebSocket):
    await monitor.register(websocket)
    try:
        while True:
            data = await websocket.receive_text()
            # Handle client messages if needed
    except:
        await monitor.unregister(websocket)

@app.get("/api/cache")
async def get_cache_info():
    return {
        "entries": [
            {
                "pageId": 1,
                "status": "Active",
                "lastAccess": "2024-01-01 12:00:00",
                "hitCount": 150,
                "dirty": True
            }
            # ... more entries
        ]
    }

@app.get("/api/pages/{page_id}")
async def get_page_details(page_id: int):
    return {
        "id": page_id,
        "content": "Page content here...",
        "metadata": {
            "created": "2024-01-01 10:00:00",
            "lastModified": "2024-01-01 11:00:00",
            "size": 4096
        }
    }

@app.get("/api/btree")
async def get_btree_structure():
    return {
        "nodes": [
            {"id": 1, "label": "Root [10, 20, 30]"},
            # ... more nodes
        ],
        "edges": [
            {"from": 1, "to": 2},
            # ... more edges
        ]
    }

# Start periodic updates
async def periodic_updates():
    while True:
        await monitor.broadcast({
            "type": "stats_update",
            "data": {
                "cacheHitRate": 85,
                "activePages": 1234,
                "memoryUsage": 456,
                "totalOps": 789000
            }
        })
        await asyncio.sleep(1)

@app.on_event("startup")
async def startup_event():
    asyncio.create_task(periodic_updates())