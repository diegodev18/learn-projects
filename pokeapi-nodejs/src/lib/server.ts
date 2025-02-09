import { createServer } from 'http';

export default function server() {
    return createServer((req, res) => {
        res.setHeader("Content-Type", "application/json");

        const { url, method } = req;
        
        if (url === "/" && method === "GET") {
            res.end(JSON.stringify({ mensage: "Bienvenido a la POKEAPI de DiegoDev18" }));
        }
        
        const regex = /^\/(\w+)\/*$/;
        const validUrl = url?.match(regex);
        if (!validUrl) {
            res.end(JSON.stringify({ mensage: `URL Invalido! Solo se requiere un parametro. Ejemplo: /pikachu` }));
        }
    })
}
