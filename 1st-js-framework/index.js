function App() {
    const people = ["Marco", "Luis", "Juan", "Pedro"];

    return createVNode(
        "ul",
        {
            style: 'display: flex; flex-direction: column; gap: 15px; margin: 0; padding-left: 20px;'
        },
        people.map((p) => person(p)),
    );
}

function person(name) {
    return createVNode(
        "li",
        {
            onClick: `console.log('Haz clickeado sobre ${name}')`,
            style: 'font-size: 18px; margin: 0;'
        },
        name
    );
}

const dom = render(App());
document.querySelector("body").appendChild(dom);
