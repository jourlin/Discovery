<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0//EN">
<HTML>
<head><style type="text/css">
<meta http-equiv="content-type" content="text/html; charset=utf-8" />
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
<title>Tied-Acronym classes</title>
</head>
<body>
<center><h1>Tied-Acronym classes</h1></center>
<span>
<?php
require("param.inc.php");

$connexion = @mysql_connect($host, $user, $mdp) ;
if ($connexion >0)
  echo "<!-- Successful connection of user $user to host $host --><br>";
else
  echo "Unsuccessful connection to host $serveur";

mysql_select_db($bd, $connexion) or die("User $user cannot connect to database $bd");
echo "<!-- User $user is connected to database $bd --><BR>";

// Where the search is to be conducted :
$request = "SELECT count(*) AS nb, journal from bibentry group by journal ORDER BY nb DESC;";
$result =  mysql_query($request);

echo "Looking for tied-acronym classes in the following document collection :";
echo "<center><table>\n";
echo "<tr><th>journal</th> <th>number of articles</tr>";
while ($row = mysql_fetch_row($result) )
	{
	echo "<tr><td>$row[1]</td><td>$row[0]</td></tr>\n";
	};
echo "</table></center><BR>\n";

// Number of unique acronyms processed 
$request = "SELECT count(*) FROM acronym";
$result =  mysql_query($request);
$row = mysql_fetch_row($result);
echo "-> Found $row[0] distinct acronyms in ";
// Number of docs processed 
$request = "SELECT count(*) FROM bibentry";
$result =  mysql_query($request);
$row = mysql_fetch_row($result);
echo "$row[0] distinct documents<BR>\n";
// Number of classes
$request = "SELECT max(classid) FROM equivclasses";
$result =  mysql_query($request);
$row = mysql_fetch_row($result);
echo "-> $row[0] distinct classes of acronyms<BR>\n";
// Number of classes containing more than one acronym
$request = "SELECT count(*) FROM (SELECT classid, count( acroid ) as nbacros, acroid FROM `equivclasses` GROUP BY classid HAVING count( acroid ) >1) as X ;";
$result =  mysql_query($request);
$row = mysql_fetch_row($result);
echo "-> $row[0] classes that contain more than one acronym<BR>\n";
// Number of classes containing more than one acronym
$request = "SELECT count(*) FROM worthyequivclasses;";
$result =  mysql_query($request);
$row = mysql_fetch_row($result);
echo "-> $row[0] classes that contain more than one acronym that occurs in more than one document : <BR>\n";

$request = "SELECT * FROM worthyequivclasses";
$result =  mysql_query($request);

echo "<table>";
echo "<tr><th>Class</th> <th>acronyms</th> <th>documents</th></tr>";
$i=1;
while ($row = mysql_fetch_row($result) )
{
	echo "<tr><td>class #".$row[0]."<br>contains ".$row[1]." acronyms<br> found in ".$row[2]." distinct documents</td>";
	$request2 = 'SELECT acroid FROM equivclasses WHERE equivclasses.classid='.$row[0].';';
	$result2 =  mysql_query($request2);
	echo "<td>";
	while ($acroid = mysql_fetch_row($result2) )
		{
		    $request3 = 'SELECT acronym.string, def FROM acronym, definitions WHERE id='.$acroid[0].' AND acronym.string=definitions.string ORDER BY freq DESC LIMIT 0,1;';
		    $result3 =  mysql_query($request3);
		    $def=mysql_fetch_row($result3);
		    echo $def[0]." : ".$def[1]."<BR>";
		    $lastacroid=$acroid[0];
		};
	echo "</td>";

	$request2 = 'SELECT DISTINCT idarticle FROM contain WHERE idacronym='.$lastacroid.';';
	$result2 =  mysql_query($request2);
	echo "<td>";
	while ($docid = mysql_fetch_row($result2) )
		{
		    $request3 = 'SELECT bibentry.author, bibentry.year, bibentry.url FROM article, bibentry WHERE article.id='.$docid[0].' AND LOCATE(article.filename, bibentry.url) <>0;';
		    $result3 =  mysql_query($request3);
		    $key=mysql_fetch_row($result3);
		    echo '<A href="'.$key[2].'">';
		    echo "- ".$key[0]." (".$key[1].")</a><BR>";
		    //echo " (".$docid[0].") <BR>";
		};
	echo "</td></tr>\n";

};
echo "</table>";

?>
</span>
</body>
</html>

