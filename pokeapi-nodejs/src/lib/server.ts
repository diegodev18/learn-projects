import { createServer } from 'http';

export default function server() {
    return createServer((req, res) => {
        res.setHeader("Content-Type", "application/json");
    
        if (req.url === "/" && req.method === "GET") {
            res.end(JSON.stringify({ mensage: "Bienvenido a la POKEAPI de DiegoDev18" }));
        }
    })
}
