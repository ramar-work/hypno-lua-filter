# home.tpl
<html>
<head>

</head>

<body>
	<ul>
	{{ #home }}
		<li>{{ .category }}</li>
		<li>{{ .header }}</li>
	{{ /home }}
	</ul>
</body>
</html>
