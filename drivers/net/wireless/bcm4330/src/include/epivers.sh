#! /bin/bash
#
# Create the epivers.h file from epivers.h.in
# 
# Epivers.h generation mechanism supports both cvs and svn based checkouts
#
# $Id: epivers.sh,v 13.30.4.1 2010/11/02 02:47:51 Exp $

BOM_PRODUCTTAG="../tools/release/producttag.txt"
NULL="/dev/null"

# Check for the in file, if not there we're probably in the wrong directory
if [ ! -f epivers.h.in ]; then
	echo No epivers.h.in found
	exit 1
fi

# TODO: Quick way to find if the version control tool is cvs or svn
# TODO: This needs to go away eventually
svn info epivers.h.in > $NULL 2>&1
if [ "$?" == "0" ]; then
   VCTOOL=SVN
else
   VCTOOL=CVS
fi

if echo "${TAG}" | grep -q "BRANCH\|TWIG"; then
	branchtag=$TAG
else
	branchtag=""
fi

# If the version header file already exists, increment its build number.
# Otherwise, create a new file.
if [ -f epivers.h ]; then
	build=`grep EPI_BUILD_NUMBER epivers.h | sed -e "s,.*BUILD_NUMBER[ 	]*,,"`
	build=`expr ${build} + 1`
	echo build=${build}
	sed -e "s,.*_BUILD_NUMBER.*,#define EPI_BUILD_NUMBER	${build}," \
		< epivers.h > epivers.h.new
	mv epivers.h epivers.h.prev
	mv epivers.h.new epivers.h
else
	# If this is a tagged build, use the tag to supply the numbers
	# Tag should be in the form
	#    <NAME>_REL_<MAJ>_<MINOR>
	# or
	#    <NAME>_REL_<MAJ>_<MINOR>_RC<RCNUM>
	# or
	#    <NAME>_REL_<MAJ>_<MINOR>_RC<RCNUM>_<INCREMENTAL>

	if [ "$VCTOOL" == "CVS" ]; then
		# This variable setting is updated by CVS upon checkout of epivers.sh
		CVSTAG='$Name: FALCON_REL_5_90_91 $'
		# Allow overriding the product tag from BOM configuration files.
		if [ -r $BOM_PRODUCTTAG ]; then
			. $BOM_PRODUCTTAG
		fi
		# Remove leading cvs "Name: " and trailing " $"
		CVSTAG=${CVSTAG/#*: /}
		CVSTAG=${CVSTAG/% $/}
	else
		if [ -n "${TAG}" ]; then
			SVNTAG=$TAG
		else
			# URL: http://svn.sj.broadcom.com/svn/wlan/branches/KIRIN_BRANCH_5_100/src/include/epivers.sh
			SVNURL=$(svn info epivers.sh | egrep "^URL: > $NULL 2>&1")
			case "${SVNURL}" in
				*/branches/*) 	SVNTAG=$(echo $SVNURL | sed -e 's%.*/branches/\(.*\)/src.*%\1%g' | xargs printf "%s")
						;;
				*/tags/*) 	SVNTAG=$(echo $SVNURL | sed -e 's%.*/tags/\(.*\)/src.*%\1%g' | xargs printf "%s")
						;;
				*/trunk/*) 	SVNTAG=$(echo $SVNURL | sed -e 's%.*/trunk/\(.*\)/src.*%\1%g' | xargs printf "%s")
						;;
				*)       	SVNTAG=""
						;;
			esac
		fi
	fi

	# TAG env var is supplied by calling makefile or build process
	#    
	# If the checkout is from a branch tag, cvs checkout or export does
	# not replace rcs keywords. In such instances TAG env variable can
	# be used (by uncommenting following line). TAG env variable format
	# itself needs to be validated for number of fields before being used.
	# (e.g: HEAD is not a valid tag, which results in all '0' values below)
	#
	VCTAG=${CVSTAG:-${SVNTAG}}

        if [ -n "$branchtag" ]; then
	   TAG=${TAG:-${VCTAG}}
        else
	   TAG=${VCTAG/HEAD/}
        fi

	# Split the tag into an array on underbar or whitespace boundaries.
	IFS="_	     " tag=(${TAG})
	unset IFS

        tagged=1
	if [ ${#tag[*]} -eq 0 ]; then
	   tag=(`date '+TOT REL %Y %m %d 0 %y'`);
	   # reconstruct a TAG from the date
	   TAG=${tag[0]}_${tag[1]}_${tag[2]}_${tag[3]}_${tag[4]}_${tag[5]}	   
	   tagged=0
	fi

	# Allow environment variable to override values.
	# Missing values default to 0
	#
	maj=${EPI_MAJOR_VERSION:-${tag[2]:-0}}
	min=${EPI_MINOR_VERSION:-${tag[3]:-0}}
	rcnum=${EPI_RC_NUMBER:-${tag[4]:-0}}

	if [ -n "$branchtag" ]; then
		[ "${tag[5]:-0}" -eq 0 ] && echo "Using date suffix for incr"
		today=`date '+%Y%m%d'`
		incremental=${EPI_INCREMENTAL_NUMBER:-${tag[5]:-${today:-0}}}
	else
		incremental=${EPI_INCREMENTAL_NUMBER:-${tag[5]:-0}}
	fi
	origincr=${EPI_INCREMENTAL_NUMBER:-${tag[5]:-0}}
	build=${EPI_BUILD_NUMBER:-0}

	# Strip 'RC' from front of rcnum if present
	rcnum=${rcnum/#RC/}
	
	# strip leading zero off the number (otherwise they look like octal)
	maj=${maj/#0/}
	min=${min/#0/}
	rcnum=${rcnum/#0/}
	incremental=${incremental/#0/}
	origincr=${origincr/#0/}
	build=${build/#0/}

	# some numbers may now be null.  replace with with zero.
	maj=${maj:-0}
	min=${min:-0}

	rcnum=${rcnum:-0}
	incremental=${incremental:-0}
	origincr=${origincr:-0}
	build=${build:-0}

	if [ ${tagged} -eq 1 ]; then
	    # vernum is 32chars max
	    vernum=`printf "0x%02x%02x%02x%02x" ${maj} ${min} ${rcnum} ${origincr}`
	else 
	    vernum=`printf "0x00%02x%02x%02x" ${tag[7]} ${min} ${rcnum}`
	fi

	# make sure the size of vernum is under 32 bits. Otherwise, truncate. The string will keep 
	# full information.
	vernum=${vernum:0:10}

	# build the string directly from the tag, irrespective of its length
	# remove the name , the tag type, then replace all _ by . 
	tag_ver_str=${TAG/${tag[0]}_}
	tag_ver_str=${tag_ver_str/${tag[1]}_}
	tag_ver_str=${tag_ver_str//_/.}

	# record tag type
	tagtype=

	if [ "${tag[1]}" = "BRANCH" -o "${tag[1]}" = "TWIG" ]; then
	    tagtype=" (TOB)"
	fi

	echo version string: "$tag_ver_str"
	echo tag type:"$tagtype"

	# OK, go do it

	echo "maj=${maj}, min=${min}, rc=${rcnum}, inc=${incremental}, build=${build}"

	sed \
		-e "s;@EPI_MAJOR_VERSION@;${maj};" \
		-e "s;@EPI_MINOR_VERSION@;${min};" \
		-e "s;@EPI_RC_NUMBER@;${rcnum};" \
		-e "s;@EPI_INCREMENTAL_NUMBER@;${incremental};" \
		-e "s;@EPI_BUILD_NUMBER@;${build};" \
		-e "s;@EPI_VERSION@;${maj}, ${min}, ${rcnum}, ${incremental};" \
		-e "s;@EPI_VERSION_STR@;${tag_ver_str};" \
		-e "s;@EPI_VERSION_TYPE@;${tagtype};" \
                -e "s;@EPI_VERSION_NUM@;${vernum};" \
		-e "s;@EPI_VERSION_DEV@;${maj}.${min}.${rcnum};" \
		< epivers.h.in > epivers.h

fi
