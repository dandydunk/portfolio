angular.module("zapit", []);
angular.module("zapit").component("listPhones", {
	templateUrl:"public/angularTemplates/listProducts.html",
	controller:function listPhonesController() {
		alert("b")
		var auctions = new Array();
		for(var i = 0; i < 20; i++) {
			auctions.push({title:'title '+i, price:i+40, remainingTime:i+10+" days"});
		}
		
		this.auctions = auctions;
		alert("a");
	}
});