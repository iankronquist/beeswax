FROM ubuntu:14.04


ENV wordpress_url https://wordpress.org/wordpress-3.0.tar.gz
RUN apt-get -y update
RUN apt-get -y install apache2 libapache2-mod-auth-mysql php5-mysql curl
RUN curl $wordpress_url > wordpress.tar.gz
RUN tar xzf wordpress.tar.gz -C /
RUN cp -r wordpress/* /var/www/html

COPY ./wp-config.php /var/www/html/wp-config.php
EXPOSE 80
ADD ./000-default.conf /etc/apache2/conf-available/000-default.conf
ADD ./httpd.conf /etc/apache2/conf/apache2.conf

CMD ["apache2", "-DFOREGROUND"]
