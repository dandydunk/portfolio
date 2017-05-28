var PlayBook = {
    QueryUser: function () {
        var id = $("#textUserId").val();
        var fName = $("#textUserFirstName").val();
        var lName = $("#textUserLastName").val();
        var phone = $("#textUserPhone").val();
        var email = $("#textUserEmail").val();
        var pass = $("#textUserPassword").val();
        var pc = $("#textUserCPass").val();

        var t = $("#tblUsersBody").html() + "<tr> <td>" + id + "</td> <td>" + fName + "</td> <td>" + lName + "</td> <td>" + phone + "</td> <td>" + email + "</td> <td>" + pass + "</td> <td>" + pc + "</td> </tr>";
        $("#tblUsersBody").html(t);
    },
    QueryProduct: function () {
        var id = $("#textProductId").val();
        var name = $("#textProductName").val();
        var description = $("#textProductDescription").val();
        var price = $("#textProductPrice").val();
        
        var t = $("#tblProductBody").html() + "<tr> <td>" + id + "</td> <td>" + name + "</td> <td>" + description + "</td> <td>" + price + "</td><td>&nbsp;</td></tr>";
        $("#tblUsersBody").html(t);
    },
    QueryPlay:function() {
        var name = $("#textPlayName").val();
        var id = $("#textPlayId").val();
        var playbookId = $("#selectPlayBook").val();
        var svgUrl = $("#textSvgUrl").val();
        var type = $("#textType").val();
        var personnel = $("#textPersonnel").val();
        var offenseFriendlyDisplay = $("#textOffenseFriendlyDisplay").val();
        var offenseName = $("#textOffenseName").val();
        var offenseValue = $("#textOffenseValue").val();
        var defenseFriendlyDisplay = $("#textDefenseFriendlyDisplay").val();
        var defenseName = $("#textDefenseName").val();
        var defenseValue = $("#textDefenseValue").val();

        var t = $("#tblRowPlays").html() + "<tr><td>"+playbookId+"</td> <td>"+id+"</td> <td>"+name+"</td> <td>"+svgUrl+"</td> <td>"+personnel+"</td> <td>"+type+"</td> <td>"+offenseFriendlyDisplay+"</td> <td>"+offenseName+"</td> <td>"+offenseValue+"</td> <td>"+defenseFriendlyDisplay+"</td> <td>"+defenseName+"</td> <td>"+defenseValue+"</td></tr>";
        $("#tblRowPlays").html(t);
    },
    QueryPlayBook:function() {
        var name = $("#textPlayBookName").val();
        var id = $("#textPlayBookId").val();

        var t = $("#tblRowPlayBooks").html()+"<tr><td>"+name+"</td> <td>"+id+"</td> <td></td></tr>";
        $("#tblRowPlayBooks").html(t);


        t = $("#selectPlayBook").html() + "<option value='"+id+"'>"+name+"</option>";
        $("#selectPlayBook").html(t);
    },
    GetUsers: function (tid) {
        var t = document.getElementById("tblUsers");
        var rows = t.rows;

        var a = new Array();
        for (var i = 1; i < rows.length; i++) {
            var row = rows[i];
            var u = new User();
            u.teamId = tid;
            u.userId = row.cells[0].innerText;
            u.firstName = row.cells[1].innerText;
            u.lastName = row.cells[2].innerText;
            u.phone = row.cells[3].innerText;
            u.email = row.cells[4].innerText;
            u.password = row.cells[5].innerText;
            u.confirmPassword = row.cells[6].innerText;

            a.push(u);
        }

        return a;
    },
    GetProducts: function (tid) {
        var t = document.getElementById("tblProducts");
        var rows = t.rows;

        var a = new Array();
        for (var i = 1; i < rows.length; i++) {
            var row = rows[i];
            var u = new Product();
            u.productId = row.cells[0].innerText;
            u.name = row.cells[1].innerText;
            u.description = row.cells[2].innerText;
            u.price = row.cells[3].innerText;
            
            a.push(u);
        }

        return a;
    },
    GetPlaysByPlayBookId: function (id) {
        var playTbl = document.getElementById("tblPlays");
        var pRows = playTbl.rows;

        var a = new Array();
        for (var i = 1; i < pRows.length; i++) {
            var row = pRows[i];
            var pbId = row.cells[0].innerText;
            if (pbId != id) {
                console.log(id + " not found");
                continue;
            }
            var pid = row.cells[1].innerText;
            var name = row.cells[2].innerText;
            var svgUrl = row.cells[3].innerText;
            var personnel = row.cells[4].innerText;
            var type = row.cells[5].innerText;
            var ofn = row.cells[6].innerText;
            var on = row.cells[7].innerText;
            var ov = row.cells[8].innerText;
            var dfn = row.cells[9].innerText;
            var dn = row.cells[10].innerText;
            var dv = row.cells[11].innerText;

            var play = new Play();
            play.playId = pid;
            play.playBookId = id;
            play.name = name;
            play.svgUrl = svgUrl;
            play.personnel = personnel;
            play.type = type;

            var o = new OffenseFormation();
            o.friendlyDisplay = ofn;
            o.name = on;
            o.value = ov;
            o.children = new Array();
            play.offenseFormation = o;

            var d = new DefenseFormation();
            d.friendlyDisplay = dfn;
            d.name = dn;
            d.value = dv;
            d.children = new Array();
            play.defenseFormation = d;

            a.push(play);
        }

        return a;
    },
    CreateTeam: function () {
        var t = new Team();
        t.teamId = $("#textTeamId").val();
        t.name = $("#textTeamName").val();
        t.createDate = $("#textCreateDate").val();
        t.expirationDate = $("#textExpirationDate").val();
        
        
        t.playbooks = new Array();

        /* get the play books */
        var pbTbl = document.getElementById("tblPlayBooks");
        var pbRows = pbTbl.rows;
        for (var i = 1; i < pbRows.length; i++) {
            var row = pbRows[i];
            var name = row.cells[0].innerText;
            var id = row.cells[1].innerText;

            var p = new PlayBookf();
            p.teamId = t.teamId;
            p.playbookId = id;
            p.plays = PlayBook.GetPlaysByPlayBookId(id);

            t.playbooks.push(p);
        }

        t.users = PlayBook.GetUsers(t.teamId);
        t.products = PlayBook.GetProducts();

        var data = JSON.stringify(t);
        
        console.log(data);
        var ajax = $.ajax({ url: "/Management/TeamManagement/CreateTeam", data: "data="+data, dataType:"json" });
        ajax.fail(function () {
            alert("ajax failed; try again.");
        });

        ajax.done(function (result) {
            console.log(result);
        });
    }
};

function Team() {
    this.teamId = null;
    this.name = null;
    this.createDate = null;
    this.expirationDate = null;
    this.playbooks = null;
    this.users = null;
    this.products = null;
}

function PlayBookf() {
    this.teamId = null;
    this.playbookId = null;
    this.playbookType = null;
    this.plays = null;
}

function PlayBookType() {
    this.friendlyDisplay = null;
    this.name = null;
    this.value = null;
    this.children = null;
}

function Play() {
    this.playId = null;
    this.playBookId = null;
    this.name = null;
    this.offenseFormation = null;
    this.defenseFormation = null;
    this.svgUrl = null;
    this.personnel = null;
    this.type = null;
}

function OffenseFormation() {
    this.friendlyDisplay = null;
    this.name = null;
    this.value = null;
    this.children = null;
 }

function DefenseFormation() {
    this.friendlyDisplay = null;
    this.name = null;
    this.value = null;
    this.children = null;
}

function User() {
   this.userId = null;
   this.teamId = null;
   this.firstName = null;
   this.lastName = null;
   this.phone = null;
   this.email = null;
   this.password = null;
   this.confirmPassword = null;
}

function Product() {
    this.productId = null;
    this.name = null;
    this.description = null;
    this.price = null;
    this.features = null;
}