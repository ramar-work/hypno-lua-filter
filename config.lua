-- config.lua
return {
	static = {
		"/assets", "/robots.txt"
	}
,	db = "yamama"
, title = "localhost"
, root = ""
,	routes = {
		["/"] = { model = "single", views = { "single" } }
	,	["books"] = { 
			model = { "books", "print-books", "edit-books" }
		, views = { "intro", "books", "outro" } 
		}
	,	["book"] = { model = { "books", "single-book" }, views = { "intro", "b", "outro" } }
	,	["home"] = { 
			system = "jaundice",
			model = "nobody", 
			views = { "a", "b", "c" } 
		}
	}
}
