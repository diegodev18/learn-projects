import { createServer } from 'http';
import pokemons from '../db/pokemons';

export default function server() {
    return createServer((req, res) => {
        res.setHeader("Content-Type", "application/json");

        const { url, method } = req;
        const { end: returnApi } = res;
        
        if (url === "/" && method === "GET") {
            returnApi(JSON.stringify({ mensage: "Bienvenido a la POKEAPI de DiegoDev18" }));
            return;
        }

        const regex = /^\/(\w+)\/*$/;
        const validUrl = url?.match(regex);
        if (!validUrl) {
            returnApi(JSON.stringify({ mensage: `URL Invalido! Solo se requiere un parametro. Ejemplo: /pikachu` }));
            return;
        }

        const pokemon = pokemons.find(({ name }) => name.toLowerCase() === validUrl[1] );
        returnApi(JSON.stringify({
            mensage: pokemon ? "Found" : "Not found",
            pokemon: pokemon ?? {}
        }));
    })
}
