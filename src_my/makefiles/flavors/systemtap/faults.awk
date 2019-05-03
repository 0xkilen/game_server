#! /usr/bin/awk -f 

BEGIN {
	printf "# thread time read seek minor major\n"
	t0 = 0
}

# pass-through lines
/^[^0-9]| - / {
	next
}

# grab log start time
t0 == 0 {
	t0 = $1
}

# query: ts pid "qs" string
/ qs / {
	printf "%u %.1f %u %.1f %u %u\n", \
		$2, ($1-t0)/1e6, q_io[$2], q_seek[$2], q_min[$2], q_maj[$2]

	q_io[$2] = 0
	q_seek[$2] = 0
	q_min[$2] = 0
	q_maj[$2] = 0
}

# disk read: ts "r" sector nr_sectors disk_major disk_minor
/ r / {
	seekrel = ($3 - curhead[$5,$6])
	seekabs = (seekrel < 0) ? -seekrel : seekrel
	curhead[$5,$6] = $3 + $4

	io += $4
	seek += seekabs
}

# fault: ts "f" pid ns address
/ f | F / {
	if (io == 0) {
		# if no I/O, count as a minor fault
		++q_min[$3]
	}
	else {
		q_io[$3] = io
		io = 0;
		q_seek[$3] = seek
		seek = 0
		++q_maj[$3]
	}
}

