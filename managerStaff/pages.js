var pages = {
	"/":{
		parent:0,
		security:1,
		name:"Dashboard",
		url:"/"
	},
	"staff":{
		parent:"/",
		security:1,
		name:"Staff",
		url:"/staff"
	},
	"employee":{
		parent:"staff",
		security:1,
		name:"Employee",
		url:"/employee"
	},
	"reports":{
		parent:"staff",
		security:1,
		name:"Reports",
		url:"/staff/reports"
	},
	"clock_in":{
		parent:"reports",
		security:1,
		name:"Clock In",
		url:"/clock_in"
	},
	"clock_out":{
		parent:"clock_in",
		security:1,
		name:"Clock Out",
		url:"/clock_out"
	},
	"save_employee": {
		parent:"staff",
		security:1,
		name:"Save Employee",
		url:"/save_employee"
	}
};

exports.NeedSecurity = function(path) {
	if(!pages[path]) return false;
	return pages[path].security;
}

exports.MakeBreadCrumb = function(current_page, crumb) {
	if(!crumb) {
		crumb = new Array();
	}

	crumb.push(pages[current_page]);
	
	var page = pages[current_page];

	if(!page || !page.parent) {
		return crumb;
	}

	exports.MakeBreadCrumb(page.parent, crumb);
	return crumb;
}