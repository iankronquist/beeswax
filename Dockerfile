FROM centos:7


ENV wordpress_url https://wordpress.org/wordpress-3.0.tar.gz
RUN yum -y install httpd php php-mysql curl
RUN curl $wordpress_url > wordpress.tar.gz
RUN tar xzf wordpress.tar.gz -C /
RUN cp -r wordpress/* /var/www/html

COPY ./wp-config.php /var/www/html/wp-config.php
EXPOSE 80
ADD ./000-default.conf /etc/httpd/sites-available/000-default.conf
ADD ./httpd.conf /etc/httpd/conf/httpd.conf

CMD ["httpd", "-DFOREGROUND"]
