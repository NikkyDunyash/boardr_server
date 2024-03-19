export function loadHtml(path, id, mode = "replace") {
    let elem = document.getElementById(id);
    let xhr = new XMLHttpRequest();
    xhr.open("GET", path, true);
    xhr.addEventListener("loadend", (event) => {
        if (xhr.status >= 400) {
            console.log("Load html error!");
        }
        if (mode == "replace") {
            elem.innerHTML = xhr.responseText;
        }
        else {
            elem.insertAdjacentHTML(mode, xhr.responseText);
        } 
    });
    xhr.send();
    return xhr;
}
