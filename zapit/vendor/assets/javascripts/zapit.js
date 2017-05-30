var ang = angular.module("zapit", []);

ang.controller("ControllerListAuctions", ["$scope", "$interval", function($scope, $interval){
	if($scope.auctions == null) {
		$scope.auctions = new Array();
		$scope.i = 0;
	}
	
	
	$interval(function(){
			$scope.auctions.push({title:'title '+$scope.i, price:$scope.i+40, remainingTime:$scope.i+10+" days"});
			($scope.i)++;
		}, 100, 20);
	
}]);