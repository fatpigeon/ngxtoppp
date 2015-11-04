"""ngxtop - ad-hoc query for nginx access log.

Usage:
    ngxtop [options]
    ngxtop [options] (print|top|avg|sum) <var> ...
    ngxtop info
    ngxtop [options] query <query> ...

Options:
    -l <file>, --access-log <file>  access log file to parse.
    -f <format>, --log-format <format>  log format as specify in log_format directive. [default: combined]
    --no-follow  ngxtop default behavior is to ignore current lines in log
                     and only watch for new lines as they are written to the access log.
                     Use this flag to tell ngxtop to process the current content of the access log instead.
    -t <seconds>, --interval <seconds>  report interval when running in follow mode [default: 2.0]

    -g <var>, --group-by <var>  group by variable [default: request_path]
    -w <var>, --having <expr>  having clause [default: 1]
    -o <var>, --order-by <var>  order of output for default query [default: count]
    -n <number>, --limit <number>  limit the number of records included in report for top command [default: 10]
    -a <exp> ..., --a <exp> ...  add exp (must be aggregation exp: sum, avg, min, max, etc.) into output

    -v, --verbose  more verbose output
    -d, --debug  print every line and parsed record
    -h, --help  print this help message.
    --version  print version information.

    Advanced / experimental options:
    -c <file>, --config <file>  allow ngxtop to parse nginx config file for log format and location.
    -i <filter-expression>, --filter <filter-expression>  filter in, records satisfied given expression are processed.
    -p <filter-expression>, --pre-filter <filter-expression> in-filter expression to check in pre-parsing phase.

Examples:
    All examples read nginx config file for access log location and format.
    If you want to specify the access log file and / or log format, use the -f and -a options.

    "top" like view of nginx requests
    $ ngxtop

    Top 10 requested path with status 404:
    $ ngxtop top request_path --filter 'status == 404'

    Top 10 requests with highest total bytes sent
    $ ngxtop --order-by 'avg(bytes_sent) * count'

    Top 10 remote address, e.g., who's hitting you the most
    $ ngxtop --group-by remote_addr

    Print requests with 4xx or 5xx status, together with status and http referer
    $ ngxtop -i 'status >= 400' print request status http_referer

    Average body bytes sent of 200 responses of requested path begin with 'foo':
    $ ngxtop avg bytes_sent --filter 'status == 200 and request_path.startswith("foo")'

    Analyze apache access log from remote machine using 'common' log format
    $ ssh remote tail -f /var/log/apache2/access.log | ngxtop -f common
"""

from __future__ import print_function
import atexit
from contextlib import closing
import curses
import logging
import os
import sqlite3
import time
import sys
import signal
import json


try:
    import urlparse
except ImportError:
    import urllib.parse as urlparse

from docopt import docopt
import tabulate

from .config_parser import detect_log_config, detect_config_path, extract_variables, build_pattern
from .utils import error_exit

# my cpp extension
import ngxtop_cpp


def count_by(d, keys):
    _keys = filter(keys, d.keys())
    return sum(d[k] for k in _keys)

Summary = ('summary',
    {
        'y': ['', ''],
        'x': [
            ['count', lambda y, h, v: count_by(v['count'], lambda _k: True)],
            ['bytes_sent', lambda y, h, v: count_by(v['bytes_sent'], lambda _k: True)],
            [lambda x: set([int(i[0])/100 for i in x['count']]),
                lambda y, h, v: count_by(v['count'], lambda _k: int(_k[0])/100 == int(h))]
        ]
    },
    ['status', ],
    ['bytes_sent']

)

Detailed = (
    'Detailed:',     # title_name
    {
        'y': ['geoip_city', lambda v, h: set([i[0] for i in v['count']])],
        'x': [
            ['count', lambda y, h, v: count_by(v['count'], lambda _k: _k[0] == y)],
            ['bytes_sent', lambda y, h, v: count_by(v['bytes_sent'], lambda _k: _k[0] == y)],
            [lambda x: set([int(i[1])/100 for i in x['count']]),
                lambda y, h, v: count_by(v['count'], lambda _k: int(_k[1])/100 == int(h) and _k[0] == y )]
        ]
    },

    ['geoip_city', 'status'],  # group by
    ['bytes_sent']  # sum_by

)

DEFAULT_QUERIES = [
    Summary,
    Detailed
]


def process(arguments):
    access_log = arguments['--access-log']
    log_format = arguments['--log-format']
    if access_log is None and not sys.stdin.isatty():
        # assume logs can be fetched directly from stdin when piped
        access_log = 'stdin'
    if access_log is None:
        access_log, log_format = detect_log_config(arguments)

    logging.info('access_log: %s', access_log)
    logging.info('log_format: %s', log_format)
    if access_log != 'stdin' and not os.path.exists(access_log):
        error_exit('access log file "%s" does not exist' % access_log)

    if arguments['info']:
        print('nginx configuration file:\n ', detect_config_path())
        print('access log file:\n ', access_log)
        print('access log format:\n ', log_format)
        print('available variables:\n ', ', '.join(sorted(extract_variables(log_format))))
        return


def main():
    args = docopt(__doc__, version='xstat 0.1')

    log_level = logging.WARNING
    if args['--verbose']:
        log_level = logging.INFO
    if args['--debug']:
        log_level = logging.DEBUG
    logging.basicConfig(level=log_level, format='%(levelname)s: %(message)s')
    logging.debug('arguments:\n%s', args)

    try:
        process(args)
    except KeyboardInterrupt:
        sys.exit(0)
