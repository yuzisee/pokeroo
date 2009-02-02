echo svn.ignore> svn.ignore

echo ece499rpt.aux>> svn.ignore
echo ece499rpt.aux.old>> svn.ignore

echo ece499rpt.toc>> svn.ignore
echo ece499rpt.lot>> svn.ignore
echo ece499rpt.lof>> svn.ignore
echo ece499rpt.toc.old>> svn.ignore
echo ece499rpt.lot.old>> svn.ignore
echo ece499rpt.lof.old>> svn.ignore

echo ece499rpt.bbl>> svn.ignore
echo ece499rpt.bbl.run>> svn.ignore

echo ece499rpt.log>> svn.ignore
echo ece499rpt.blg>> svn.ignore


svn propset svn:ignore -F svn.ignore .
