<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0//EN">
<HTML>
<head>
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
echo "<title>List of definition for class ".$_GET['class']."</title>";
?>
</head>
<body>

<?
require("param.inc.php");
$class=$_GET['class'];

echo "<center><h1>List of definition for acronym ".$_GET['class']."</h1></center>\n";

$connexion = @pg_connect("host=$host user=$user dbname=$dbname password=$mdp") ;
if ($connexion)
  echo "<!-- Successful connection of user $user to host $host<br> -->";
else
  echo "Unsuccessful connection to host $serveur";

$request = "SELECT e.definition, count(o.docid) as count FROM stemseqcat as s, stemseqcatcontain as c, entities as e, occurrences as o WHERE s.string='$class' AND s.scid=c.scid AND c.entid=e.entid AND e.entid=o.entid GROUP BY e.definition ORDER BY count DESC;";
echo "<!-- $request -->";

$result =  pg_query($request);

echo "<table align=center>";
echo "<tr> <th align=right>Definition</th> <th align=right>Occurences</th></tr>";
while ($row = pg_fetch_row($result) )
 echo "<tr align=right><td>$row[0]</td><td>$row[1]</td></tr>";
echo "</table>";

?>
</body>
</html>

