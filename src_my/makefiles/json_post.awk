BEGIN {
	print "["
}

END {
	print "]"
}

/["{}]/ {
	print $0
}

{
}

