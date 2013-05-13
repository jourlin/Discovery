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
<title>List of unpublished acronym definition pairs A-C</title>
</head>
<body>
<center><h1>List of unpublished acronym definition pairs A-C</h1></center>
<span>
<?php
require("param.inc.php");

$connexion = @pg_connect("host=$host user=$user dbname=$dbname password=$mdp") ;
if ($connexion)
  echo "<!-- Successful connection of user $user to host $host --><br>";
else
  echo "Unsuccessful connection to host $host";

$classa=$_GET['classa'];
if(!isset($classa))
	die("This script expects one parameter (class a)");
$classa_req = "SELECT scid FROM stemseqcat WHERE string='$classa';";
$classa_res = pg_query($classa_req);
$classaid=pg_fetch_row($classa_res);
	
$mainrequest = "SELECT DISTINCT classc, nbc FROM listofhypothesis WHERE classa='$classaid[0]' ORDER by nbc DESC";
$mainresult =  pg_query($mainrequest);

echo "<table>";
echo "<tr><th>id</th> <th>Class A</th> <th>Class C</th> <th>Class B (most specific link)</th></tr>";
$i=1;
while ($classc = pg_fetch_row($mainresult) )
{
	// Class A
	echo "<tr rowspan=2>";
	echo "<td>".$i++."</td>";
	echo '<td align="center">';
	echo "<b>$classa</b> <a href=./def.php?class=".urlencode($classa).">*</a><BR>";
	echo "</td>\n";
	// Class C
	$classc_req = "SELECT string FROM stemseqcat WHERE scid='$classc[0]';";
	$classc_res = pg_query($classc_req);
	echo '<td align="center">';
	$classcstring=pg_fetch_row($classc_res);
	echo "<b>$classcstring[0]</b> <a href=./def.php?class=".urlencode($classcstring[0]).">*</a><BR>";
	echo "Class $classcstring[0] occurs in <a href=./docs.php?class=".urlencode($classcstring[0]).">$classc[1] docs</a><BR>\n";
	echo "<a href=./exploreac.php?classa=".urlencode($classa)."&classc=".urlencode($classcstring[0]).">Explore A-C</a><BR>\n";
	echo "</td>\n";
	// Class B
	$request = "SELECT classb, nbb FROM listofhypothesis WHERE classa='$classaid[0]' AND classc='$classc[0]' ORDER by nbb ASC LIMIT 1";
	$result =  pg_query($request);
	$classbid = pg_fetch_row($result);
	$classb_req = "SELECT string FROM stemseqcat WHERE scid='$classbid[0]';";
	$classb_res = pg_query($classb_req);
	echo '<td align="center">';
	$classbstring=pg_fetch_row($classb_res);
	echo "<b>$classbstring[0]</b> <a href=./def.php?class=".urlencode($classbstring[0]).">*</a><BR>";
	echo "Class $classbstring[0] occurs in <a href=./docs.php?class=".urlencode($classbstring[0]).">$classbid[1] docs</a><BR>\n";
	echo "</td>\n";

	echo "</tr>\n";
	
};

echo "</table>";

?>
</span>
</body>
</html>

