#! gawk -f
# script that post-processes "cl.exe /showIncludes ..." output to a dependency file

BEGIN {
	exitcode = 0

	if (ARGC != 3) {
		printf "usage: %s -f {this script} depfile={dependency filepath} target={target filepath}", ARGV[0]
		exit 1;
	}

	# we want to ignore paths inside the system dirs, e.g. under the VS root
	# get these from the same environment variable the compiler uses
	split(ENVIRON["INCLUDE"], ignore_pfxs, ";")
	if (ENVIRON["MSDEV_HOME"] != "") {
		ignore_pfxs[0] = ENVIRON["MSDEV_HOME"]
	}
	for (i in ignore_pfxs) {
		# Make sure we always have a trailing slash
		ignore_pfxs[i] = (ignore_pfxs[i] "/")
		# Convert backslashes to forward slashes
		gsub(/\\/, "/", ignore_pfxs[i])
		# Fold multiple slashes down to just one
		gsub(/\/+/, "/", ignore_pfxs[i])
		ignore_pfxs_len[i] = length(ignore_pfxs[i])
	}

	# VS seems to lower-case some of the include names, so...
	IGNORECASE = 1
}

/^Note: including file:/ { # rule: found an include file message
	fn = gensub("^Note: including file: *(.*)$","\\1","")
	gsub(/\\/, "/", fn)
	found = 0
	for (i in ignore_pfxs) {
		if (substr(fn, 1, ignore_pfxs_len[i]) == ignore_pfxs[i]) {
			found = 1
			break
		}
	}
	if (!found) files[fn]++
	next
}

# pass through errors from compiler.
# this form is for the Microsoft tools
/ : .*error C/ {
	exitcode = 1
}

# pass through errors from compiler.
# this form is for the Intel tools
/: .*error .*: / {
	exitcode = 1
}

# drop "current file being compiled" messages
/^[[:graph:]]+\.(c|cc|cxx|cpp)$/ {
	next
}

{ # default rule - print to output for compiler messages
	print
}

END {
	# set up the dependencies
	printf("$(call print_if,$(PRINT_D_FILE_INCLUDES),Including %s)\n", depfile) > depfile
	printf("%s: ",target) > depfile
	for (file in files) {
		printf("\\\n\t%s ",file) > depfile
	}
	# create dummy targets for each, in case they're missing
	printf("\n\n") > depfile
	for (file in files) {
		printf("%s:\n\n",file) > depfile
	}

	exit exitcode
}

