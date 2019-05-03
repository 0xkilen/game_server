#
# Takes a sequence of parent child relationships, and prints out a bottom-up
# topo sort of the DAG. Optionally, the variable 'root' can be set on the
# command line to only output the part of the DAG rooted at the specified node
#
# Each relationship should come on a line on stdin, where the first word is the
# name of the parent, and subsequent words are the children, in no particular
# order.
#

# C[] maps the parent to a space-joined list of children

# S[] has marks for nodes we have started visiting
# F[] has marks for nodes we have finished visiting

# Stack[] is the current call stack of keys, used only for printing an error
# message of there's a loop.

# Note: In awk, all local variables must be declared as arguments to the function.
# Here, "key" is the actual argument, "children" is just a local variable.
function visit(key, children)
{
	if (key in F) return
	Stack[length(Stack)] = key
	if (key in S) {
		print "Found cycle involving " key > "/dev/stderr"
		for (i = 0; i < length(Stack); i++) {
			if (Stack[i] == key) {
				break;
			}
		}
		printf "%s", Stack[i] > "/dev/stderr"
		for (i++; i < length(Stack); i++) {
			printf " -> %s", Stack[i] > "/dev/stderr"
		}
		print "" > "/dev/stderr"
		exit 1
	}
	S[key] = 1
	if (key in C) {
		split(C[key], children)
		for (i in children) {
			visit(children[i])
		}
	}
	F[key] = 1
	print key
	delete Stack[length(Stack)-1]
}

# Join with spaces
function joinarr(arr, result)
{
	result = ""
	for (i in arr) {
		if (result != "") result = result " "
		result = result arr[i]
	}
	return result
}

/./ {
	split($0, words)
	first = words[1]
	delete words[1]
	C[first] = joinarr(words)
}

END {
	if (root) {
		visit(root)
	} else {
		for (i in C) {
			visit(i)
		}
	}
}
