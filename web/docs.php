<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0//EN">
<HTML>
<head>
<?
require("param.inc.php");
$class=$_GET['class'];
$def=$_GET['def'];
?>

<meta http-equiv="Content-type" content="text/html"; charset=utf-8">
<style type="text/css">
<!--
table {
	border-width: 2px 2px 2px 2px;
	border-spacing: 2px;
	border-style: outset outset outset outset;
	border-color: blue blue blue blue;
	border-collapse: collapse;
	background-color: white;
}
table th {
	border-width: 4px 4px 4px 4px;
	padding: 1px 1px 1px 1px;
	border-style: inset inset inset inset;
	border-color: green green green green;
	background-color: white;
	-moz-border-radius: 12px 12px 12px 12px;
}
table td {
	border-width: 4px 4px 4px 4px;
	padding: 1px 1px 1px 1px;
	border-style: inset inset inset inset;
	border-color: green green green green;
	background-color: white;
	-moz-border-radius: 12px 12px 12px 12px;
}
-->
</style>
<?
if(isset($class))
	echo "<title>List of documents containing the definition of class $class </title>";
else
if(isset($class1)&&isset($class2))
	echo "<title>List of documents containing the definition of class $class1 and $class2 </title>";
else
if(isset($def))
	echo "<title>List of documents containing the definition of acronym $def </title>";
else
	die ("Unrecognized parameters sent to script");
?>
</head>
<body>
<?
if(isset($class))
	echo "<center><h1>List of documents containing the definition of class $class </h1></center>";
else
if(isset($class1)&&isset($class2))
	echo "<center><h1>List of documents containing the definition of class $class1 and $class2 </h1></center>";
else
if(isset($def))
	echo "<center><h1>List of documents containing the definition of acronym $def </h1></center>";
else
	die ("Unrecognized parameters sent to script");

$connexion = @pg_connect("host=$host user=$user dbname=$dbname password=$mdp") ;
if ($connexion)
  echo "<!-- Successful connection of user $user to host $host --><br>";
else
  die("Unsuccessful connection to host $serveur");
if(isset($class))
	$request = "SELECT DISTINCT title, author, journal, year, url  FROM stemseqcat as s, stemseqcatcontain as c, occurrences as o, documents as d, bibentries as b WHERE s.string='$class' AND s.scid=c.scid AND c.entid=o.entid AND o.docid=d.docid AND d.docname=substring(b.url from '%/#".'"'."%#".'"'."' for '#') ORDER BY year DESC";
else
if(isset($def))
	$request = "SELECT DISTINCT title, author, journal, year, url  FROM entities as e, occurrences as o, documents as d, bibentries as b WHERE e.definition='$def' AND e.entid=o.entid AND o.docid=d.docid AND d.docname=substring(b.url from '%/#".'"'."%#".'"'."' for '#') ORDER BY year DESC";
else
	die ("Unrecognized parameters sent to script");
echo "<!-- $request -->";

$result =  pg_query($request);

echo "<table border=2>";
echo "<tr> <th>Title</th><th>Authors</th><th>Journal</th><th>Year</th><th>link</th> </tr>";
while ($row = pg_fetch_row($result) )
 echo "<tr><td>$row[0]</td><td>$row[1]</td><td>$row[2]</td><td>$row[3]</td><td><a href=$row[4]>full text</a></td></tr>";
echo "</table>";

?>
</body>
</html>

