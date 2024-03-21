import { loadHtml } from "/js/load.js";


const DEF_COMMENTS_NUM = 10;
const MAX_COMMENT_LENGTH = 1024;

class CustomCache {
    static MAX_CACHE_SIZE = 20;
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
    let offset = 0, comp = "g", order = "desc", mode="afterbegin";
    
    let loadNewComments = (() => {
        if (!scroll) {
            let ids = document.getElementsByClassName("comment-id");
            if (ids.length) {
                offset = ids[0].innerText;
            }
            getComments(DEF_COMMENTS_NUM, offset, comp, order, mode);
        } 
    });
    webSocket = new WebSocket("wss://" + location.host + "/comments");
    webSocket.addEventListener("open", loadNewComments);
    webSocket.addEventListener("message", loadNewComments);
    webSocket.onclose = function () {
        setTimeout(setupWebSocket, 1000);
    };
}


// Events handlers
// mode = "replace" | "afterbegin" | "beforeend"
function getComments(num, offset, comp, order, mode = "afterbegin") {
    if (!document.getElementById("loader")) {
        let loader = document.createElement("div");
        loader.setAttribute("id", "loader");
        loader.className = "loader";
        document.getElementById("comments").insertAdjacentElement("afterbegin", loader);
    }
    let xhr = loadHtml("/get_comments?num=" + num + "&offset=" + offset + "&comp=" + comp + "&order=" + order,
        "comments", mode);
    xhr.addEventListener("loadend", (event) => {
        document.getElementById("loader").remove();
    });
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
    }
}


function scrollComments(event) {
    let ids = document.getElementsByClassName("comment-id");
    let offset = 0, comp = "ge", order = "desc", mode = "replace";
    scroll = true;

    if (!document.getElementById("button_live")) {
        let buttonLive = document.createElement("button");
        buttonLive.setAttribute("id", "button_live");
        buttonLive.className = "btn-scroll btn-top-bottom";
        buttonLive.appendChild(document.createTextNode("Live"));
        buttonLive.addEventListener("click", scrollComments);
        document.getElementById("scroll_buttons").insertAdjacentElement("afterbegin", buttonLive);
    }

    if (event.target.id == "button_down") {
        offset = ids[Math.floor(ids.length / 3)].innerText;
        comp = "le";
    }
    else if (event.target.id == "button_up") {
        offset = ids[Math.floor(2 * ids.length / 3)].innerText;
        order = "asc";
    }
    else if (event.target.id == "button_live") {
        scroll = false;
        document.getElementById("button_live").remove();
    }
    else if (event.target.id == "button_bottom") {
        order = "asc";
    }
    getComments(DEF_COMMENTS_NUM, offset, comp, order, mode);
}


function loadAllComments() {
    let ids = document.getElementsByClassName("comment-id");
    let offset = 0;
    if (ids.length) {
        offset = ids[ids.length - 1].innerText;
    }
    getComments("all", offset, "l", "desc", "beforeend");
}


function postComment() {
    let inputComment = document.getElementById("input_comment").value;
    if (inputComment.length < 1 || inputComment.length > MAX_COMMENT_LENGTH) {
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



let scroll = false;

let pfpCache = new CustomCache();

let webSocket = null;
setupWebSocket();

document.getElementById("button_down").addEventListener("click", scrollComments);
document.getElementById("button_up").addEventListener("click", scrollComments);
document.getElementById("button_bottom").addEventListener("click", scrollComments);

document.getElementById("button_loadall").addEventListener("click", loadAllComments);

document.getElementById("input_comment").addEventListener("keyup", (event) => {
    if ((event.keyCode == 10 || event.keyCode == 13)
        && event.ctrlKey) {
        postComment();
    }
});
document.getElementById("input_comment").setAttribute("placeholder",
    `No more than ${MAX_COMMENT_LENGTH} bytes, enjoy...`);
document.getElementById("button_post").addEventListener("click", postComment);







