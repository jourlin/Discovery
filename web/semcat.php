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
<title>List of semantic categories</title>
</head>
<body>
<center><h1>List of semantic categories</h1></center>
<span>
<?php
require("param.inc.php");
echo "<center><table><tr><td><a href=./index.php>back to scientific gaps</a></td><td>....</td><td><a href=./tagger.php>back to tagger</a></td></tr></table></center><br>\n";
$ip=$_SERVER["REMOTE_ADDR"];
// Check permissions
if($ip=="194.57.216.212"||$ip=="147.100.66.242"||$ip=="147.100.18.219"||$ip=="127.0.0.1")
	{
		echo $_SERVER["REMOTE_ADDR"]." is <b>allowed</b> to edit semantic categories<br>\n";
		
	}
else
	{
		echo $_SERVER["REMOTE_ADDR"]." is <b>not allowed</b> to edit semantic categories<br>\n";
		unset($_POST['semcat']);
		unset($_POST['newname']);
		unset($_POST['rename']);
	};

$connexion = @pg_connect("host=$host user=$user dbname=$dbname password=$mdp") ;
if ($connexion)
  echo "<!-- Successful connection of user $user to host $host --><br>";
else
  echo "Unsuccessful connection to host $host";

// process POST entries
if(isset($_POST['semcat'])&&!isset($_POST['newname'])&&!isset($_POST['newmark']))
{
	// delete semcat

	$updatereq="DELETE FROM isofsemcat USING semcat WHERE id=secid AND name='".$_POST['semcat']."';";
	$updateres =pg_query($updatereq);
	if(!$updateres)
	{
		echo "Failed to delete semantic category";
		exit; 
	};
	$updatereq="DELETE FROM semcat WHERE name='".$_POST['semcat']."';";
	$updateres =pg_query($updatereq);
	if(!$updateres)
	{
		echo "Failed to delete semantic category";
		exit; 
	};
	// write on tagger log
	$semcatname=$_POST['semcat'];
	$logreq="INSERT INTO taggerlog (ip, date, time, command) VALUES ('$ip',current_date, current_time, 'del ($semcatname)' );";
	$logres =  pg_query($logreq);
	if(!$logres)
	{
		echo "Failed to insert into tagger log";
		exit; 
	};
	
	echo "<center><font color=#808000>The semantic class <i>".$_POST['semcat']."</i> has been deleted.</font></center><br>\n";
}
elseif(isset($_POST['semcat'])&&isset($_POST['newname']))
{
	// rename semcat
	$updatereq="UPDATE semcat SET name ='".$_POST['newname']."' WHERE name='".$_POST['semcat']."';";
	$updateres =pg_query($updatereq);
	if(!$updateres)
	{
		echo "Failed to update semantic category";
		exit; 
	};
	// write on tagger log
	$semcatname=$_POST['semcat'];
	$newsemcatname=$_POST['newname'];
	$logreq="INSERT INTO taggerlog (ip, date, time, command) VALUES ('$ip',current_date, current_time, 'ren ($semcatname, $newsemcatname)' );";
	$logres =  pg_query($logreq);
	if(!$logres)
	{
		echo "Failed to insert into tagger log";
		exit; 
	};

	echo "<center><font color=#808000>The semantic class <i>".$_POST['semcat']."</i> has been renamed as ".$_POST['newname']."</font></center><br>\n";
}
/* elseif(isset($_POST['semcat'])&&isset($_POST['newmark']))
{
	// remark semcat
	$updatereq="UPDATE semcat SET mark ='".$_POST['newmark']."' WHERE name='".$_POST['semcat']."';";
	$updateres =pg_query($updatereq);
	if(!$updateres)
	{
		echo "Failed to update semantic category";
		exit; 
	};
	// write on tagger log
	$semcatname=$_POST['semcat'];
	$newsemcatname=$_POST['newmark'];
	$logreq="INSERT INTO taggerlog (ip, date, time, command) VALUES ('$ip',current_date, current_time, 'mark($semcatname, $newsemcatname)' );";
	$logres =  pg_query($logreq);
	if(!$logres)
	{
		echo "Failed to insert into tagger log";
		exit; 
	};

	echo "<center><font color=#808000>The semantic class <i>".$_POST['semcat']."</i> has been marked as ".$_POST['newmark']."</font></center><br>\n";
}
*/

// Where the search is to be conducted :
$request = "SELECT name FROM semcat ORDER BY name ASC;";
$result =  pg_query($request);
if(!$result)
{
	echo "Failed to access to table semcat";
	exit; 
}
echo "<center>Existent semantic classes :";
echo "<table>\n";
echo "<tr><th>action</th><th>Sem. class</th><th>Acronym classes (mark)</th></tr>";
while ($row = pg_fetch_row($result) )
	{
	echo "<tr><td align=center>";
	echo '<FORM method=post action="'.$SERVER['PHP_SELF'].'">';
	echo '<INPUT type=hidden name="semcat" value="'.$row[0].'">'."\n";
	echo '<INPUT type="submit" value="delete"></FORM>';
	echo '<font size=1>or rename:</font><FORM method=post action="'.$SERVER['PHP_SELF'].'">';
	echo '<INPUT type=text name="newname">'."\n";
	echo '<INPUT type=hidden name="semcat" value="'.$row[0].'">'."\n";
	echo '<INPUT type="submit" value="rename"></FORM>'."</td>\n";
	// name
	echo "<td align=center>$row[0]</td>";
	/* mark
	echo '<td><FORM method=post action="'.$SERVER['PHP_SELF'].'">';
	echo '<select name="newmark" size=1>';
	for($i=1; $i<=5; $i++)
	{
		echo '<option value="'.$i.'"';
		if($i==$row[1])
			echo ' selected="selected"';
		echo '>'.$i.'</option '."\n"; 
	};
	echo '<INPUT type=hidden name="semcat" value="'.$row[0].'">'."\n";
	echo '</select><INPUT type="submit" value="change"></FORM></td>'."\n"; */

	echo "<td align=center>";
	$stemseqcatreq="SELECT string, mark FROM stemseqcat, semcat, isofsemcat WHERE semcat.name='".$row[0]."' AND semcat.id=isofsemcat.secid AND isofsemcat.stcid=stemseqcat.scid ORDER BY string ASC;";
	$stemseqcatres=pg_query($stemseqcatreq);
	while ($ssc = pg_fetch_row($stemseqcatres))
		{
			echo "$ssc[0] ($ssc[1]), ";
		}
	echo "</td></tr>\n";
	};
echo "</table></center><BR>\n";

?>
</span>
</body>
</html>

