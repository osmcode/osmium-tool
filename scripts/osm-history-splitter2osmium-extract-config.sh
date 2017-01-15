#!/bin/sh
#
#  Convert a config file in the format for the osm-history-splitter into a
#  config file for the "osmium extract" command.
#

if [ -n "$1" ]; then
    exec <$1
fi

echo '{'
echo '    "extracts": ['
while read output type region; do
    echo '        {'
    echo "            \"output\": \"$output\","
    if [ "$type" = "BBOX" ]; then
        echo "            \"bbox\": [$region]"
    elif [ "$type" = "OSM" -o "$type" = "POLY" ]; then
        lctype=`echo $type | tr 'A-Z' 'a-z'`
        cat << __END__
            "polygon": {
                "file_name": "$region",
                "file_type": "$lctype"
            }
__END__
    fi
    echo '        },'
done

echo '    ]'
echo '}'

