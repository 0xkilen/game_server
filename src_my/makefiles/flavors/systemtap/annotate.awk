#! /usr/bin/awk -f 

# Correlate page faults and seeks against indexes
# this script takes the output from system tap and from the dgraph when the
# --xlog <fn> parameter is used and produces a report of the page faults that
# each query causes

BEGIN {
	id = "program"
	tbegin["0x"] = "program"
	tend["0x"] = "0xfffffffffff"
	t0 = 0
	UNKNOWN = "(unknown)"
}

# note - making this function actually return the value
# seems to result in a gawk memory leak, so return the
# result through a global
function find_id(addr) {
	found_id = UNKNOWN
	while (sub(/0x0/, "0x", addr)) { }
	for (i in tbegin) {
		if ((addr >= i) &&
			(addr <= tend[i])) {
			found_id = tbegin[i]
			break
		}
	}
}

function flush(tid, latency) {
	printf "%8u  %12.1f ms    d=%5.1f ms, read=%6u, seek=%8.1e, dist=%10d, cursors=%6u : ", tid, ($1-t0)/1e6, latency, iosectors, ioseekabs, ioseekrel, cursors
	iosectors = 0
	ioseekabs = 0
	ioseekrel = 0
	cursors = 0
}

#{
#	fflush("/dev/stdout")
#}

# pass-through lines
/^[^0-9]| - / {
	print $0
	next
}

# grab log start time
t0 == 0 {
	t0 = $1
}

# index definition: ts pid "ix" index_name begin end
/ ix / {
	lastIndexName = $4
	addr = $5
	while (sub(/0x0/, "0x", addr)) { }
	if (addr < tend["0x"]) {
		tend["0x"] = addr
	}
}

# index blocks definition: ts pid "tl" begin end
/ tl / {
	tbegin[$4] = lastIndexName " index"
	tend[$4] = $5
}

# index interpolation index definition: ts pid "it" level begin end
/ it / {
	tbegin[$5] = lastIndexName " interp-" $4
	tend[$5] = $6
}

# base tuple definition: ts pid "tv" begin end
/ tv / {
	tbegin[$4] = lastIndexName " data"
	tend[$4] = $5
}

# base OOLS definition: ts pid "to" begin end
/ to / {
	tbegin[$4] = lastIndexName " OOLS"
	tend[$4] = $5
}

# query: ts pid "qs" string
/ qs / {
	flush($2, 0)
	printf "end of query: %s\n", lastqs[$2]
	printf "%8u  %12.1f ms    query summary: t=%.1f ms tdisk=%.1f%% read=%6u seek=%8.1e m=%u %.1f ms M=%u %.1f ms, cursors=%u\n", \
		$2, ($1-t0)/1e6, \
		($1-lastq[$2])/1e6, \
		100.0 * (q_mint[$2]+q_majt[$2])/($1-lastq[$2]), \
		q_io, q_seek, q_min[$2], q_mint[$2]/1e6, q_maj[$2], q_majt[$2]/1e6, q_cursors[$2]

	if (indextimes_count > 0) {
		tempfile = ("sorttmp." PROCINFO["pid"])
		command = "sort -k2,2nr > " tempfile

		for (idx in indextimes) {
			split(idx, idxparts, SUBSEP)
			printf "%8u         %7.1f ms  read=%6u seek=%8.1e M=%3u   %s\n", idxparts[1], indextimes[idx]/1e6, \
				indexio[idx], indexseek[idx], indexfaults[idx], idxparts[2] | command
			delete idxparts
		}
		delete indextimes
		delete indexio
		delete indexseek
		delete indexfaults
		indextimes_count = 0;
		close(command)

		printf "%8u       Index detail:\n", $2
		while ((getline pr < tempfile) > 0)
			print pr
		close(tempfile)
		system("rm " tempfile)
	}

	flush($2, 0)
	q_io = 0
	q_seek = 0
	q_min[$2] = 0
	q_mint[$2] = 0
	q_maj[$2] = 0
	q_majt[$2] = 0
	q_cursors[$2] = 0

	printf "start of query: %s\n", $4
	lastq[$2] = $1
	lastqs[$2] = $4
}

# disk read: ts "r" sector nr_sectors disk_major disk_minor
/ r / {
	seekrel = ($3 - curhead[$5,$6])
	seekabs = (seekrel < 0) ? -seekrel : seekrel
	curhead[$5,$6] = $3 + $4

	iosectors += $4
	ioseekrel += seekrel
	ioseekabs += seekabs
	q_io += iosectors
	q_seek += seekabs
}

# fault: ts "f" pid ns address
/ f | F / {
	if (iosectors == 0) {
		# if no I/O, count as a minor fault
		++q_min[$3]
		q_mint[$3] += $4
	}
	else {
		find_id($5)
		# summarize by index name & phase
		split(found_id, idparts)
		++indextimes_count
		idxid = idparts[1] " (" phase[$3] ")"
		indextimes[$3,idxid] += $4
		++indexfaults[$3,idxid]
		indexio[$3,idxid] += iosectors
		indexseek[$3,idxid] += seekabs
		delete idparts
		flush($3, $4/1e6)
		printf "fault %s (%s)\n", found_id, phase[$3]
		++q_maj[$3];
		q_majt[$3] += $4
	}
}

# phase: ts pid "ph" description
/ ph / {
	phase[$2] = $4
}

# cursor creation: ts pid "ac" index_address
/ ac | pm / {
	++cursors
	++q_cursors[$2]
}

# cursor copy: ts pid "cc" address
/ cc / {
}

# cursor delete: ts pid "~c" address
/ ~c / {
}


