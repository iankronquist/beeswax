FROM centos:7


ENV wordpress_url https://wordpress.org/wordpress-3.0.tar.gz
RUN yum -y install httpd php php-mysql curl
RUN curl $wordpress_url > wordpress.tar.gz
RUN tar xzf wordpress.tar.gz -C /var/www

COPY ./wp-config.php /var/www/wp-config.php

CMD ["httpd-foreground"]
