

function solicitar() {
    const figura = document.getElementById("figure").value;
    const parte = document.getElementById("part").value;

    let url = `http://127.0.0.1:1234/figura/${figura}/${parte}`;

    fetch(url)
        .then(res => res.text())
        .then(data => {
            const lineas = data.trim().split("\n");

            let html = "<h3>Resultado</h3>";
            html += "<table>";
            html += "<tr><th>Cantidad</th><th>Pieza</th></tr>";

            lineas.forEach(linea => {
                if (linea.includes(" : ")) {
                    const partes = linea.split(" : ");
                    html += `<tr><td>${partes[0]}</td><td>${partes[1]}</td></tr>`;
                } else if (linea.includes("Total")) {
                    html += `</table><p class="total">${linea}</p>`;
                }
            });

            document.getElementById("resultado").innerHTML = html;
        })
        .catch(err => {
            console.error(err);
            document.getElementById("resultado").innerText = "Error de conexión";
        });
}