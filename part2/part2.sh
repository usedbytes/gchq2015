#/bin/bash

for url in `./get_urls.py`; do
	echo -n "."
	wget -qO out.html $url
	grep -qi "Sorry" out.html
	if [ "$?" -ne "0" ]; then
		echo "Gotcha: $url"
		exit 0;
	fi;
	sleep 0.$[ ( $RANDOM % 10 ) + 1 ]s
done;
