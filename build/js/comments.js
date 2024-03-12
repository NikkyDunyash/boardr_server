import { loadHtml } from "/js/load.js";


class CustomCache {
    static MAX_CACHE_SIZE = 50;
    static MAX_FREQ = 1000;
    constructor() {
        this.keyVal = new Map();
        this.keyFreq = new Map();
    }
    has(key) {
        return this.keyVal.has(key);
    }
    get(key) {
        let freq = this.keyFreq.get(key);
        if (freq < CustomCache.MAX_FREQ) {
            this.keyFreq.set(key, freq + 1);
        }
        return this.keyVal.get(key);
    }
    set(key, val) {
        if (this.keyVal.size == CustomCache.MAX_CACHE_SIZE) {
            let keys = Array.from(this.keyFreq.keys());
            keys.sort((a, b) => this.keyFreq.get(a) - this.keyFreq.get(b));
            this.keyVal.delete(keys[0]);
            this.keyFreq.delete(keys[0]);
        }
        this.keyVal.set(key, val);
        this.keyFreq.set(key, 1);
    }
}


// WebSocket setup
function setupWebSocket() {
    webSocket = new WebSocket("wss://" + location.host + "/comments");
    webSocket.addEventListener("open", (event) => {
        getComments();
    });
    webSocket.addEventListener("message", (event) => {
        getComments();
        console.log("Message from server: ", event.data);
    });
    webSocket.onclose = function () {
        setTimeout(setupWebSocket, 1000);
    };
}


// Events handlers
function getComments() {
    let xhr = loadHtml("/get_comments?num=" + num + "&offset=" + offset + "&comp=" + comp + "&order=" + order, "comments");
    xhr.addEventListener("loadend", getPfp);
}


function getPfp() {
    let comments = document.getElementsByClassName("comment");
    for (let i = 0; i < comments.length; i++) {
        let username = comments[i].getElementsByTagName("b")[0].innerText;
        if (pfpCache.has(username)) {
            comments[i].getElementsByClassName("pfp")[0].src = "data:image/gif;base64," + pfpCache.get(username);
            continue;
        }
        let xhr = new XMLHttpRequest();
        xhr.open("GET", `/username=${username}/pfp`, false);
        xhr.send();
        comments[i].getElementsByClassName("pfp")[0].src = "data:image/gif;base64," + xhr.responseText;
        pfpCache.set(username, xhr.responseText);
        // console.log(pfpCache.keyFreq);
    }
}


function scrollComments(event) {
    let ids = document.getElementsByClassName("comment-id");
    if (event.target.id == "button_down") {
        offset = ids[Math.floor(ids.length / 3)].innerText;
        comp = "le";
        order = "desc";
    }
    else if (event.target.id == "button_up") {
        offset = ids[Math.floor(2 * ids.length / 3)].innerText;
        comp = "ge";
        order = "asc";
    }
    else if (event.target.id == "button_top") {
        offset = 0;
        comp = "ge";
        order = "desc";
    }
    else if (event.target.id == "button_bottom") {
        offset = 0;
        comp = "ge";
        order = "asc";
    }
    getComments();
}


function postComment() {
    let inputComment = document.getElementById("input_comment").value;
    console.log(inputComment);
    if (inputComment.length < 1 || inputComment.length > 256) {
        return;
    }
    let xhr = new XMLHttpRequest();
    xhr.open("POST", "/post_comment", true);
    xhr.setRequestHeader("Content-Type", "text/html; charset=utf-8");
    xhr.onreadystatechange = function () {
        if (this.readyState !== 4) { return; }
        if (Math.floor(this.status / 100) !== 2) {
            console.log("No inputComment posted");
            return;
        }
    }
    xhr.send(inputComment);
    webSocket.send("new input_comment");
    document.getElementById("input_comment").value = "";
}



let num = 10;
let offset = 0;
let comp = "ge";
let order = "desc";

let pfpCache = new CustomCache();

let webSocket = null;
setupWebSocket();

document.getElementById("button_down").addEventListener("click", scrollComments);
document.getElementById("button_up").addEventListener("click", scrollComments);
document.getElementById("button_bottom").addEventListener("click", scrollComments);
document.getElementById("button_top").addEventListener("click", scrollComments);

document.getElementById("input_comment").addEventListener("keyup", (event) => {
    if ((event.keyCode == 10 || event.keyCode == 13)
        && event.ctrlKey) {
        postComment();
    }
});

document.getElementById("button_post").addEventListener("click", postComment);






