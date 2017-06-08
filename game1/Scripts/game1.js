var GameOne = {
    Ctx: null,
    GameCanvas: null,
    BufferCanvas: null,
    BufferCtx: null,
    Characters:new Array(),
    Init: function () {
        $("#gameCanvasBuffering").hide();

        GameOne.GameCanvas = document.getElementById("gameCanvas");
        GameOne.Ctx = GameOne.GameCanvas.getContext("2d");

        GameOne.BufferCanvas = document.getElementById("gameCanvas");
        GameOne.BufferCtx = GameOne.GameCanvas.getContext("2d");

        GameOne.InitCharacters();
        GameOne.GameLoop();
    },
    InitCharacters: function () {
        var c = new Character();
        GameOne.Characters.push(c);
    },
    DrawCharacters: function(){

    },
    DrawBackground: function () {
        GameOne.BufferCtx.fillStyle = "black";
        GameOne.BufferCtx.fillRect(0, 0, GameOne.GameCanvas.width, GameOne.GameCanvas.height);
    },
    DrawFrameTime: function () {
        GameOne.BufferCtx.font = "20px sans-serif";
        GameOne.BufferCtx.fillStyle = "red";  // set fill color to red
        GameOne.BufferCtx.fillText(new Date(), 0, GameOne.GameCanvas.height - 40);
    },
    DrawFrame:function() {
        GameOne.Ctx.putImageData(GameOne.BufferCtx.getImageData(0, 0, GameOne.GameCanvas.width, GameOne.GameCanvas.height), 0, 0);
    },
    GameLoop: function () {
        setTimeout(GameOne.GameLoopOne, 100);
        //console.log("game loop running");
    },
    GameLoopOne: function () {
        GameOne.DrawBackground();
        GameOne.DrawCharacters();
        GameOne.DrawFrameTime();
        GameOne.DrawFrame();
        GameOne.GameLoop();
    }
};

function Character() {

}

Character.prototype.Draw = function () {

}

$(document).ready(function () {
    GameOne.Init();
});