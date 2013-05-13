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
<title>List of acronym categories</title>
</head>
<body>
<center><h1>List of acronym categories</h1></center>
<span>
<?php
require("param.inc.php");
echo "<center><table><tr><td><a href=./index.php>back to scientific gaps</a></td><td>....</td><td><a href=./semcat.php>go to category edition</a></td></tr></table></center><br>\n";
$ip=$_SERVER["REMOTE_ADDR"];
// Check permissions
if($ip=="194.57.216.212"||$ip=="147.100.18.219"||$ip=="147.100.66.242"||$ip=="127.0.0.1")
	{
		echo $_SERVER["REMOTE_ADDR"]." is <b>allowed</b> to edit semantic categories<br>\n";
		
	}
else
	{
		echo $_SERVER["REMOTE_ADDR"]." is <b>not allowed</b> to edit semantic categories<br>\n";
		unset($_POST['newcat']);
		unset($_POST['oldcat']);
		unset($_POST['acrocat']);
	};

$connexion = @pg_connect("host=$host user=$user dbname=$dbname password=$mdp") ;
if ($connexion)
  echo "<!-- Successful connection of user $user to host $host --><br>";
else
  echo "Unsuccessful connection to host $host";

// process POST entries
if(isset($_GET['split']))
	$split=$_GET['split'];
else
	unset($split);

if(isset($_POST['oldcat']))
	$semcatname=$_POST['oldcat'];
if(isset($_POST['newcat'])&&$_POST['newcat']!="")
	$semcatname=$_POST['newcat'];

if(isset($_POST['acrocat']))
	$stemcatname=$_POST['acrocat'];

if(isset($_POST['mark']))
		$mark=$_POST['mark'];
	else
		$mark=1;
if(isset($_POST['acrocat']))
	{	
		$updatemarkreq="UPDATE stemseqcat SET mark='$mark' WHERE string='$stemcatname';";
		$updatemarkres =  pg_query($updatemarkreq);
		if(!$updatemarkres)
		{
			echo "Warning fail to mark the acronym class<br>\n";
		};
		echo "<center><font color=#808000>The acronym class <i>".$_POST['acrocat']."</i> has been marked $mark.</font></center><br>\n";
	}

if(isset($_POST['newcat'])&&$_POST['newcat']!="")
{
	// Set a acronym category to a new semantic category
	
	$insertnewsemcatreq="INSERT INTO semcat (name) VALUES ('".$_POST['newcat']."');";
	$insertnewsemcatres =  pg_query($insertnewsemcatreq);
	if(!$insertnewsemcatres)
	{
		echo "Warning fail to insert new semantic category<br>\n";
		echo "It might be because it already exits<br>\n";
	};
	echo "<center><font color=#808000>The new semantic class <i>".$_POST['newcat']."</i> has been created.</font></center><br>\n";
	// write on tagger log
	$logreq="INSERT INTO taggerlog (ip, date, time, command) VALUES ('$ip',current_date, current_time, 'newcat(".$_POST['newcat'].")' );";
	$logres =  pg_query($logreq);
	if(!$logres)
	{
		echo "Failed to insert into tagger log";
		exit; 
	};
};
if(isset($stemcatname))
{
	// Get Acronym Category's id
	$getstemcatnameidreq="SELECT scid FROM stemseqcat WHERE string='".$stemcatname."';";
	$getstemcatnameidres=pg_query($getstemcatnameidreq);
	if(!$getstemcatnameidres)
	{
		echo "Failed to find the semantic category";
		exit; 
	}
	else
		$stemcatid=pg_fetch_row($getstemcatnameidres);
	// Get Semantic Category's id
	$getsemcatnameidreq="SELECT id FROM semcat WHERE name='".$semcatname."';";
	$getsemcatnameidres=pg_query($getsemcatnameidreq);
	if(!$getsemcatnameidres)
	{
		echo "Failed to find the semantic category";
		exit; 
	}
	else
		$semcatid=pg_fetch_row($getsemcatnameidres);

	// find out if stemcat is already attached to one semcat
	$attachedreq="SELECT COUNT(*) FROM isofsemcat WHERE stcid='".$stemcatid[0]."';";
	$attachedres=pg_query($attachedreq);
	if(!$attachedres)
		{
			echo "Unable to access table isofsemcat";
			exit; 
		}
	$numattached=pg_fetch_row($attachedres);
	if($numattached[0]>1)
		{
			echo "Warning : '$stemcatname' is set to more than one category !<br>";
		};
	if($numattached[0]>=1)
		{
			$updatereq="UPDATE isofsemcat SET secid ='".$semcatid[0]."' WHERE stcid='".$stemcatid[0]."';";
			$updateres =pg_query($updatereq);
			if(!$updateres)
			{
				echo "Failed to update semantic category";
				exit; 
			};
		}
	else
	{
		$insertnewsemcatreq="INSERT INTO isofsemcat (stcid, secid) VALUES ('".$stemcatid[0]."', '".$semcatid[0]."');";
		$insertnewsemcatres =  pg_query($insertnewsemcatreq);
		if(!$insertnewsemcatres)
		{
			echo "Failed to insert new semantic category";
			exit; 
		}
	}
	// write on tagger log
	$logreq="INSERT INTO taggerlog (ip, date, time, command) VALUES ('$ip',current_date, current_time, 'tag($stemcatname, $semcatname, $mark)' );";
	$logres =  pg_query($logreq);
	if(!$logres)
	{
		echo "Failed to insert into tagger log";
		exit; 
	};

	echo "<center><font color=#808000><i>".$stemcatname."</i> has been set to semantic class <i>".$semcatname."</i></font></center><br>\n";
}
if(isset($_GET['numberofclasses']))
	$numberofclasses=$_GET['numberofclasses'];
else
	$numberofclasses=10;
if(isset($_GET['fromclass']))
	$fromclass=$_GET['fromclass'];
else
	$fromclass=0;
if($fromclass==0)
	$fromclassstr="first";
elseif($fromclass==1)
	$fromclassstr="second";
elseif($fromclass==2)
	$fromclassstr="third";
else 
	$fromclassstr=($fromclass+1)."<sup>th</sup>";

echo "<center><i>Displaying $numberofclasses most frequent classes, starting from the $fromclassstr most frequent class<br></i></center></br>\n";
echo "<center>";
if($fromclass>=$numberofclasses)
	echo "<a href=".$SERVER['PHP_SELF']."?numberofclasses=".$numberofclasses."&fromclass=".($fromclass-$numberofclasses).">Previous $numberofclasses classes</a> - ";
echo "<a href=".$SERVER['PHP_SELF']."?numberofclasses=".$numberofclasses."&fromclass=".($fromclass+$numberofclasses).">Next $numberofclasses classes</a>";
echo "</center></br>";

// Where the search is to be conducted :
$request = "SELECT DISTINCT string, count(docid) as nb, mark FROM stemseqcat, stemseqcatcontain, occurrences WHERE ";
if(!isset($split))
	 $request = $request."stemseqcat.length=2 AND ";
else
	 $request = $request."stemseqcat.string LIKE '%$split' AND ";
$request = $request."stemseqcat.scid=stemseqcatcontain.scid AND stemseqcatcontain.entid=occurrences.entid GROUP BY stemseqcat.string, stemseqcat.mark  ";

$request = $request."ORDER by nb DESC LIMIT ".$numberofclasses." OFFSET ".$fromclass.";";
$result =  pg_query($request);
if(!$result)
{
	echo "Failed to access to acronym's classes information";
	exit; 
}
echo "<center>Acronym classes that occur in hypothesis :";
echo "<table>\n";
echo "<tr><th>class (#docs)<br><font size=1>(first year-last year)</font></th><th>Acronyms (#docs)</th><th>Semantic Category<br></th></tr>";
while ($row = pg_fetch_row($result) )
	{
	// First and last years of occurences
	$yearreq="SELECT min(year), max(year)  FROM stemseqcat as s, stemseqcatcontain as c, occurrences as o, documents as d, bibentries as b WHERE s.string='$row[0]' AND s.scid=c.scid AND c.entid=o.entid AND o.docid=d.docid AND d.docname=substring(b.url from '%/#".'"'."%#".'"'."' for '#')";
	$yearres =  pg_query($yearreq);
	if(!$yearres)
	{
		echo "Failed to access years";
		exit; 
	};
	$year=pg_fetch_row($yearres);
	echo "<tr><td align=right>$row[0] (<a href=./docs.php?class=".urlencode($row[0]).">$row[1]</a>)<br>($year[0]-$year[1])";
	echo "<br><br><i><a href=./tagger.php?split=".urlencode($row[0]).">split this class</a></i>\n";
	// mark 
	echo '<br><br>mark:<form method=post action="'.$SERVER['PHP_SELF'].'">'.'<select name="mark" size=1>';
	
	for($i=1; $i<=5; $i++)
		{
			echo '<option value="'.$i.'"';
			if($i==$row[2])
				echo ' selected="selected"';
			echo '>'.$i.'</option '."\n"; 
		}
	echo "</select>\n";

	echo "</td>";

	echo "<td><font size=1><i>";

	// Find class' acronym definitions 	
	$request="SELECT e.definition, count(o.docid) as count FROM stemseqcat as s, stemseqcatcontain as c, entities as e, occurrences as o WHERE s.string='$row[0]' AND s.scid=c.scid AND c.entid=e.entid AND e.entid=o.entid GROUP BY e.definition ORDER BY count DESC;";
	$resultdef =  pg_query($request);
	if(!$resultdef)
		{
		echo "Failed to access definitions";
		exit; 
		};
	while ($def = pg_fetch_row($resultdef) )
	{
		echo $def[0]." (<a href=./docs.php?def=".urlencode($def[0]).">".$def[1]."</a>), ";
	}
	$request="SELECT name, mark FROM semcat, stemseqcat, isofsemcat WHERE stemseqcat.string='$row[0]' AND isofsemcat.stcid=stemseqcat.scid AND semcat.id=isofsemcat.secid;";
	$resultsemcat =  pg_query($request);
	if(!$resultsemcat)
		$semcat[0]="undefined";
	$semcat = pg_fetch_row($resultsemcat);
	if($semcat=="")
		$semcat[0]="undefined";		
	// Find class' semantic category
	echo '</i></font></td>';
	echo '<td align=center>';
	echo '<br><font size=1>select an existent category:</font>';
	echo '<select name="oldcat" size=1>'."\n";
	$request="SELECT name FROM semcat ORDER BY name ASC;";
	$resultsemcat =  pg_query($request);
	if(!$resultsemcat)
		{
		echo "Failed to access table semcat";
		exit; 
		};
	echo '<option value="'.$semcat[0].'" selected="selected"'."><i>$semcat[0]</i></option>\n";
	while ($name = pg_fetch_row($resultsemcat) )
		if($name[0]!==$semcat[0])		
			echo '<option value="'.$name[0].'">'."$name[0]</option>\n";
		
	echo '</select>';
	// enter a new category
	echo '<br><font size=1>or enter a new category:</font>';
	echo '<INPUT type=text name="newcat">'."\n";
	echo '<INPUT type=hidden name="acrocat" value="'.$row[0].'">'."\n";
	echo '<INPUT type="submit" value="OK"></FORM>'."\n";
	echo "</td></tr>\n";
	};
echo "</table></center><BR>\n";
echo "<center>";
if($fromclass>=$numberofclasses)
	echo "<a href=".$SERVER['PHP_SELF']."?numberofclasses=".$numberofclasses."&fromclass=".($fromclass-$numberofclasses).">Previous $numberofclasses classes</a> - ";
echo "<a href=".$SERVER['PHP_SELF']."?numberofclasses=".$numberofclasses."&fromclass=".($fromclass+$numberofclasses).">Next $numberofclasses classes</a>";
echo "</center></br>";

?>

</span>
</body>
</html>

