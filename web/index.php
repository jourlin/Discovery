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
echo "<center><table><tr><td><a href=./tagger.php>to semantic categories tagger</a></td></tr></table></center><br>\n";
$connexion = @pg_connect("host=$host user=$user dbname=$dbname password=$mdp") ;
if ($connexion)
  echo "<!-- Successful connection of user $user to host $host --><br>";
else
  echo "Unsuccessful connection to host $host";

// Where the search is to be conducted :
$request = "SELECT count(*) AS nb, journal from bibentries group by journal ORDER BY nb DESC;";
$result =  pg_query($request);
if(!$result)
{
	echo "Failed to access to table 'bibentries'";
	exit; 
}
echo "<center>This program processed the following document collection :";
echo "<table>\n";
echo "<tr><th>journal</th> <th>number of articles</tr>";
while ($row = pg_fetch_row($result) )
	{
	echo "<tr><td>$row[1]</td><td>$row[0]</td></tr>\n";
	};
echo "</table></center><BR>\n";

// Number of unique acronyms processed 
$request = "SELECT count(*) FROM entities";
$result =  pg_query($request);
$row = pg_fetch_row($result);
echo "It processed $row[0] distinct acronyms forms in ";
// Number of unique acronyms processed 
$request = "SELECT count(*) FROM documents";
$result =  pg_query($request);
$row = pg_fetch_row($result);
echo " $row[0] distinct documents. \n";
// Number of classes
$request = "SELECT count(*) FROM stemseqcat WHERE length=1";
$result =  pg_query($request);
$row = pg_fetch_row($result);
echo "It found $row[0] distinct classes of acronyms (sets of acronyms that terminates by a unique stem). \n";
echo "Below are the list of <i>unpublished</i> relations between a class A and a class C<BR>\n";
echo "The relations are ranked according to the number of documents in which each class A occurs<BR>\n";
echo "Only the less frequent class B and the most frequent class C are shown<BR>\n";

// Results filter
echo "<br><center>Filter the results using manually created semantic categories :</center>";
echo '<FORM method="GET" action="'.$SERVER['PHPSELF'].'">';
echo "<center><table>";
echo "<tr><th>Class A</th><th>Class C</th><th>Class B</th></tr>";
echo "<tr>\n";

$cat[0]=urldecode($_GET['cat0']);
$cat[1]=urldecode($_GET['cat1']);
$cat[2]=urldecode($_GET['cat2']);
$mark[0]=urldecode($_GET['mark0']);
$mark[1]=urldecode($_GET['mark1']);
$mark[2]=urldecode($_GET['mark2']);

for($i=0;$i<3;$i++) // For each class
{
	if(!isset($cat[$i])||($cat[$i]==""))
		$cat[$i]="anysemcat";
	if(!isset($mark[$i])||($mark[$i]==""))
		$mark[$i]=1;
	echo "<td>";
	echo 'relevance greater or equal to <SELECT name="mark'.$i.'">';
	for($j=1;$j<=5;$j++)
	{		
		echo "<option value=$j ";
		if($mark[$i]==$j)
			echo ' selected="selected"';
		echo ">$j</option>\n";
	};
	echo "</SELECT>\n";
	echo '<br>category :<SELECT name="cat'.$i.'">';
	echo '<option value="anysemcat"';
	if($cat[i]=="anysemcat")
		echo ' selected="selected"';
	echo '>Any Semantic Category</option>';
	$semcatreq = "SELECT DISTINCT name FROM semcat ORDER by name ASC";
	$semcatres =  pg_query($semcatreq);
	while ($row = pg_fetch_row($semcatres) )
	{		
		echo '<option value="'.urlencode($row[0]).'" ';
		if($cat[$i]==$row[0])
			echo ' selected="selected"';
		echo '>'.$row[0].'</option>';
	};
	echo "</SELECT>\n";
	echo "</td>";
}
echo "</tr>\n";
echo "</table>";
echo '<INPUT type="submit" value="Filter the results">'."\n";
echo "</center>\n";
echo "</FORM>\n";	

$filteredLoH="(SELECT * FROM listofhypothesis ";
// needed tables 
for($i=0;$i<3;$i++)
{
	if($mark[$i]>1)
		$filteredLoH=$filteredLoH.", stemseqcat as ssc$i";
	if($cat[$i]!="anysemcat")
		$filteredLoH=$filteredLoH.", isofsemcat as iosc$i, semcat as sc$i";
}
$filteredLoH=$filteredLoH." WHERE TRUE";
$letter[0]='a'; $letter[1]='c'; $letter[2]='b'; 
for($i=0;$i<3;$i++)
{
	if($mark[$i]>1)
		$filteredLoH=$filteredLoH." AND class".$letter[$i]."=ssc$i.scid AND ssc$i.mark>=".$mark[$i];
	if($cat[$i]!="anysemcat")
	{
		$filteredLoH=$filteredLoH." AND class".$letter[$i]."=iosc$i.stcid AND iosc$i.secid=sc$i.id ";
		$filteredLoH=$filteredLoH." AND sc$i.name LIKE '".$cat[$i]."%'";
	}
}
$filteredLoH=$filteredLoH.") as FLoH";
//echo "Filtered list of Hypothesis :".$filteredLoH."\n<br>";


$mainrequest = "SELECT DISTINCT classa, nba FROM $filteredLoH ORDER by nba DESC LIMIT 1000";
$mainresult =  pg_query($mainrequest);
echo "<center><table>";
echo "<tr><th>id</th> <th>Class A</th> <th>Class C</th> <th>Class B (most specific link)</th></tr>";
$i=1;
while ($classaid = pg_fetch_row($mainresult) )
{
	// Class A
	echo "<tr rowspan=2>";
	echo "<td>".$i++."</td>";
	$classa_req = "SELECT string FROM stemseqcat WHERE scid='$classaid[0]';";
	$classa_res = pg_query($classa_req);
	echo '<td align="center">';
	$classastring=pg_fetch_row($classa_res);
	echo "<b>$classastring[0]</b> <a href=./def.php?class=".urlencode($classastring[0]).">*</a><BR>";
	//$GGstring[$col] = strtr($defClass[0]," ", "+");
	echo "Class $classastring[0] occurs in <a href=./docs.php?class=".urlencode($classastring[0]).">$classaid[1] docs</a><BR>\n";
	echo "<a href=./explorea.php?classa=".urlencode($classastring[0]).">explore this class</a><BR>\n";
	
	
	//echo '<center>check:<a href=http://www.google.fr/search?q=%2B"'.$GGstring[$col].'"><img align=top src="./images/google.png" alt="check with Google"></a></center>';	
	echo "</td>\n";
	// Class C
	$request = "SELECT classc, nbc FROM $filteredLoH WHERE classa='$classaid[0]' ORDER by nbc DESC LIMIT 1";
	$result =  pg_query($request);
	$classcid = pg_fetch_row($result);
	$classc_req = "SELECT string FROM stemseqcat WHERE scid='$classcid[0]';";
	$classc_res = pg_query($classc_req);
	echo '<td align="center">';
	$classcstring=pg_fetch_row($classc_res);
	echo "<b>$classcstring[0]</b> <a href=./def.php?class=".urlencode($classcstring[0]).">*</a><BR>";
	//$GGstring[$col] = strtr($defClass[0]," ", "+");
	echo "Class $classcstring[0] occurs in <a href=./docs.php?class=".urlencode($classcstring[0]).">$classcid[1] docs</a><BR>\n";
	//echo '<center>check:<a href=http://www.google.fr/search?q=%2B"'.$GGstring[$col].'"><img align=top src="./images/google.png" alt="check with Google"></a></center>';	
	echo "</td>\n";
	// Class B
	$request = "SELECT classb, nbb FROM $filteredLoH WHERE classa='$classaid[0]' AND classc='$classcid[0]' ORDER by nbb ASC LIMIT 1";
	$result =  pg_query($request);
	$classbid = pg_fetch_row($result);
	$classb_req = "SELECT string FROM stemseqcat WHERE scid='$classbid[0]';";
	$classb_res = pg_query($classb_req);
	echo '<td align="center">';
	$classbstring=pg_fetch_row($classb_res);
	echo "<b>$classbstring[0]</b> <a href=./def.php?class=".urlencode($classbstring[0]).">*</a><BR>";
	//$GGstring[$col] = strtr($defClass[0]," ", "+");
	echo "Class $classbstring[0] occurs in <a href=./docs.php?class=".urlencode($classbstring[0]).">$classbid[1] docs</a><BR>\n";
	//echo '<center>check:<a href=http://www.google.fr/search?q=%2B"'.$GGstring[$col].'"><img align=top src="./images/google.png" alt="check with Google"></a></center>';	
	echo "</td>\n";

	echo "</tr>\n";
	//echo '<tr><td style="border:none" colspan=4><center>check A+C:<a href=http://www.google.fr/search?q=%2B"'.$GGstring[0].'"+%2B"'.$GGstring[2].'"><img align=top src="./images/google.png" alt="check with Google"></a>.....check A+B:<a href=http://www.google.fr/search?q=%2B"'.$GGstring[0].'"+%2B"'.$GGstring[1].'"><img align=top src="./images/google.png" alt="check with Google"></a>.....check B+C:<a href=http://www.google.fr/search?q=%2B"'.$GGstring[1].'"+%2B"'.$GGstring[2].'"><img align=top src="./images/google.png" alt="check with Google"></a></center><br><br></td></tr>'."\n";
	
};

echo "</table></center>";

?>
</span>
</body>
</html>

