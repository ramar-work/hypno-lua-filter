# home.tpl
<html>
<head>

</head>

<body>
	<ul>
	{{ #results }}
		<li>{{ .category }}</li>
		<li>{{ .header }}</li>
	{{ /results }}
	</ul>
</body>
</html>
