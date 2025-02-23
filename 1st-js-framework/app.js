
function createVNode(name, attributes, ...args) {
    const children = args.length > 0 ? [].concat(...args) : null;
    return { name, attributes, children };
}

function render(vNode) {
    if (typeof vNode === "string" || typeof vNode === "number") {
        return document.createTextNode(vNode);
    }

    const node = document.createElement(vNode.name);

    const attributes = vNode.attributes || {};

    Object.keys(attributes).forEach((attribute) => {
        if (attribute.startsWith('on') && typeof attributes[attribute] === "function") {
            const event = attribute.slice(2);
            node.addEventListener(event, attributes[attribute]);
            return;
        }
        node.setAttribute(attribute, attributes[attribute]);    
    });

    (vNode.children || []).forEach((child) => {
        node.appendChild(render(child));
    });

    return node;
}
