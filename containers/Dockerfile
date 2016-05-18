FROM centos:7


ENV wordpress_url https://wordpress.org/wordpress-3.0.tar.gz
RUN rpm --rebuilddb 
RUN yum -y install httpd php php-mysql curl
RUN curl $wordpress_url > wordpress.tar.gz
RUN tar xzf wordpress.tar.gz -C /
RUN cp -r wordpress/* /var/www/html
RUN rm wordpress.tar.gz

COPY ./configs/wp-config.php /var/www/html/wp-config.php
EXPOSE 80
ADD ./configs/000-default.conf /etc/httpd/sites-available/000-default.conf
ADD ./configs/httpd.conf /etc/httpd/conf/httpd.conf
RUN chown apache /var/www/html -R

CMD ["httpd", "-DFOREGROUND"]
