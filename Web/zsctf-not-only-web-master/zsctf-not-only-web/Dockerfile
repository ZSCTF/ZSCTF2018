FROM php:5.6.31-apache
ENV APACHE_CONFDIR /etc/apache2

RUN echo "[my_fast_framework]\nextension=/var/www/html/.extension/my_fast_framework.so\n" > /usr/local/etc/php/conf.d/xdebug.ini

RUN { \
		echo '<FilesMatch \.php$>'; \
		echo '\tSetHandler application/x-httpd-php'; \
		echo '</FilesMatch>'; \
		echo; \
		echo 'DirectoryIndex disabled'; \
		echo 'DirectoryIndex index.php index.html'; \
		echo; \
		echo '<Directory /var/www/>'; \
		echo '\tOptions -Indexes'; \
		echo '\tAllowOverride All'; \
		echo '</Directory>'; \
	} | tee "$APACHE_CONFDIR/conf-available/docker-php.conf"

ADD src.tar.gz /var/www/html/
RUN chown -R root:root /var/www/html
COPY flag /

RUN { \
	chown root:root /flag; \
	chmod 644 /flag; \
}
