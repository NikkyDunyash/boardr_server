export function loadHtml(path, id) {
    let xhr = new XMLHttpRequest();
    xhr.open("GET", path, true);
    xhr.addEventListener("loadend", (event) => {
        if (xhr.status >= 400) {
            console.log("Load html error!");
        }
        document.getElementById(id).innerHTML = xhr.responseText;
    });
    xhr.send();
    return xhr;
}
