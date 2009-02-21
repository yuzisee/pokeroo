echo svn.ignore> svn.ignore

echo Debug>> svn.ignore
echo Release>> svn.ignore

svn propset svn:ignore -F svn.ignore .
