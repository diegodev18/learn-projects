import { createServer } from 'http';
import pokemons from '../db/pokemons';

export default function server() {
    return createServer((req, res) => {
        res.setHeader("Content-Type", "application/json");

        const { url, method } = req;
        
        if (url === "/" && method === "GET") {
            res.end(JSON.stringify({ mensage: "Bienvenido a la POKEAPI de DiegoDev18" }));
            return;
        }

        const regex = /^\/(\w+)\/*$/;
        const validUrl = url?.match(regex);
        if (!validUrl) {
            res.end(JSON.stringify({ mensage: `URL Invalido! Solo se requiere un parametro. Ejemplo: /pikachu` }));
            return;
        }

        const pokemon = pokemons.find(({ id, name, type }) => {
            if (name.toLowerCase() === validUrl[1]) {
                return true;
            } else if (!isNaN(Number(validUrl[1])) && parseInt(validUrl[1]) === id) {
                return true;
            } else if (type.find(t => t === validUrl[1])) {
                return true;
            }
            return false;
        });
        res.end(JSON.stringify({
            mensage: pokemon ? "Found" : "Not found",
            pokemon: pokemon ?? {}
        }));
    })
}
