class WebSocketClient {
    constructor() {
        this.connect();
        this.handlers = new Map();
    }

    connect() {
        this.ws = new WebSocket('ws://localhost:8080/monitor');
        
        this.ws.onopen = () => {
            console.log('Connected to server');
        };
        
        this.ws.onmessage = (event) => {
            const data = JSON.parse(event.data);
            if (this.handlers.has(data.type)) {
                this.handlers.get(data.type)(data);
            }
        };
        
        this.ws.onclose = () => {
            console.log('Connection closed, trying to reconnect...');
            setTimeout(() => this.connect(), 5000);
        };
    }

    subscribe(type, handler) {
        this.handlers.set(type, handler);
    }

    unsubscribe(type) {
        this.handlers.delete(type);
    }
}