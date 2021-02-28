-- config.lua
return {
	static = {
		"/assets", "/robots.txt"
	}
,	db = "yamama"
, title = "localhost"
, root = ""
,	routes = {
		default = { model = "nobody", views = { "a", "b", "c" } }
	,	home = { model = "nobody", views = { "a", "b", "c" } }
	,	catering = { model = "nobody", views = { "a", "b", "c" } }
	}
}
