const MAX_PFP_SIZE = 10 * 1024 * 1024;


function checkSame(elem1, elem2, msgElem, disElem = null) {
    if (elem1.value == elem2.value && elem1.value) {
        msgElem.style.color = "green";
        msgElem.innerHTML = "matching";
        if (disElem) { disElem.disabled = false; }
    } else if (elem1.value != elem2.value && (elem1.value || elem2.value)) {
        msgElem.style.color = "red";
        msgElem.innerHTML = "not matching";
        if (disElem) { disElem.disabled = true; }
    }
    else {
        if (disElem) { disElem.disabled = true; }
    }
}


function checkSamePassword() {
    checkSame(document.getElementById("password"),
        document.getElementById("confirm_password"),
        document.getElementById("check_message"),
        document.getElementById("submit"));
}


function pfpUpload() {
    if (document.getElementById("pfp_upload").files.length !== 1) {
        return;
    }
    let pfpFileObj = document.getElementById("pfp_upload").files[0];
    if (pfpFileObj.size > MAX_PFP_SIZE) {
        alert("Large pfp chosen, max size is 10MB!");
        return;
    }
    let reader = new FileReader();
    let showProgress = ((event) => {
        let ending = "";
        if (event.type == "load") { ending = "ed" }
        document.getElementById("imgLoadLog").textContent =

            `${event.type}${ending}: ${Number(event.loaded / 1024 ** 2).toFixed(2)} MB\n`;
    });
    reader.addEventListener("loadstart", () => {
        document.getElementById("submit").disabled = true;
    });
    reader.addEventListener("progress", (event) => {
        showProgress(event);
    });
    reader.addEventListener("load", (event) => {
        let pfpPreview = document.getElementById("pfp_preview");

        pfpPreview.setAttribute("class", "pfp");
        pfpPreview.src = "data:image/gif;base64," + btoa(reader.result);

        form.append("pfp", btoa(reader.result));

        showProgress(event);

        document.getElementById("submit").disabled = false;
        if (document.getElementById("cancel_pfp_button") === null) {
            buttonCancel = document.createElement("button");
            buttonCancel.setAttribute("id", "cancel_pfp_button");
            buttonCancel.appendChild(document.createTextNode("Cancel"));
            buttonCancel.addEventListener("click", pfpCancel);
            document.getElementById("cancel_pfp").appendChild(buttonCancel);
        }
    });
    reader.readAsBinaryString(pfpFileObj);
}


function pfpCancel() {
    form.delete("pfp");
    document.getElementById("pfp_upload").value = "";
    document.getElementById("pfp_preview").src = "";
    document.getElementById("imgLoadLog").textContent = "";
    document.getElementById("cancel_pfp_button").remove();
}


function sendRegForm() {
    const url = "/reg"
    let username = document.getElementById("username").value;
    let password = document.getElementById("password").value;

    if (username == "") {
        console.log(username);
        alert("Specify username!");
        return;
    }
    form.append("username", username);
    form.append("password", password);

    xhr = new XMLHttpRequest();
    xhr.open("POST", url, true);
    xhr.onreadystatechange = function () {
        if (this.readyState !== 4) { return; }
        if (Math.floor(this.status / 100) !== 2) {
            window.location.href = "/pages/invalid_login.html";
            return;
        }
        window.location.href = "/index";
    }
    xhr.send(form);
    form = new FormData();
}


function changePfp() {
    const url = "/change_pfp"
    if (!form.has("pfp")) { return; }
    xhr = new XMLHttpRequest();
    xhr.open("POST", url, true);
    xhr.onreadystatechange = function () {
        if (this.readyState !== 4) { return; }
        if (Math.floor(this.status / 100) !== 2) {
            alert("Pfp change error!");
        }
        window.location.href = "/index";
    }
    xhr.send(form);
    form = new FormData();
}

let form = new FormData(); 