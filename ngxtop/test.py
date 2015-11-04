import ngxtop_cpp


def main():
    access_log_name = 'test.log'
    args = '{"--no-follow": true, "--pre-filter" : ""}'
    pattern_str = '$remote_addr - $remote_user [$time_local] ' \
                  '"$request" $status $body_bytes_sent "$http_referer" ' \
                  '"$http_user_agent" "$http_x_forwarded_for" $request_time ' \
                  '"$geoip_city" "$geoip_region_name" "$geoip_country_name" ' \
                  '"$upstream_addr" $upstream_response_time'
    group_sum_methods = {
        "summary": (['status', 'geoip_city'], ['bytes_sent'])
    }
    ngxtop_cpp.run(access_log_name, args, pattern_str, group_sum_methods)

    r = ngxtop_cpp.get_records()
    print r


if __name__ == '__main__':
    main()
