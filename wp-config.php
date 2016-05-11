<?php
/**
 * The base configurations of the WordPress.
 *
 * This file has the following configurations: MySQL settings, Table Prefix,
 * Secret Keys, WordPress Language, and ABSPATH. You can find more information
 * by visiting {@link http://codex.wordpress.org/Editing_wp-config.php Editing
 * wp-config.php} Codex page. You can get the MySQL settings from your web host.
 *
 * This file is used by the wp-config.php creation script during the
 * installation. You don't have to use the web site, you can just copy this file
 * to "wp-config.php" and fill in the values.
 *
 * @package WordPress
 */

// ** MySQL settings - You can get this info from your web host ** //
/** The name of the database for WordPress */
define('DB_NAME', getenv('DB_ENV_MYSQL_DATABASE'));

/** MySQL database username */
define('DB_USER', getenv('DB_ENV_MYSQL_USER'));

/** MySQL database password */
define('DB_PASSWORD', getenv('DB_ENV_MYSQL_PASSWORD'));

/** MySQL hostname */
define('DB_HOST', 'db');

/** Database Charset to use in creating database tables. */
define('DB_CHARSET', 'utf8');

/** The Database Collate type. Don't change this if in doubt. */
define('DB_COLLATE', '');

/**#@+
 * Authentication Unique Keys and Salts.
 *
 * Change these to different unique phrases!
 * You can generate these using the {@link https://api.wordpress.org/secret-key/1.1/salt/ WordPress.org secret-key service}
 * You can change these at any point in time to invalidate all existing cookies. This will force all users to have to log in again.
 *
 * @since 2.6.0
 */
define('AUTH_KEY',         'UCS%1WnBL|;?X L7B^+Zkc:|};;d3tmL/oNy8NjmLw/| R& hAJvQ!e.zRk.q|e:');
define('SECURE_AUTH_KEY',  'vBd#qym90O}Sv]D+t(sP2;z h<UJ-G=:hq!9EV0h4P0:Tk=2W/k_w {4hGP$^zjv');
define('LOGGED_IN_KEY',    'wq0/vh3iLm~mB%8$tuD}M2gHR@h)+v/6Z1X%6=~6v1d(g&}wxTp/IARsPQO1q,oK');
define('NONCE_KEY',        'q>,Oz^<(/%RNq)&25s.~ete>nO_sJSNUPW[W&FZtyWr&of{3eTCIlZb[. %X*XQt');
define('AUTH_SALT',        'wQW,6K?0K`Ls#^+.^:f~E qU&E[vr~B4[_s@ [:~[5%hpJk~9${[4<$M_@G7~1G(');
define('SECURE_AUTH_SALT', 'Z;WkU44[1lyixhS6<+~3Yxz.MqaBOY;SIkV_gP-+:/^yk W^aIc,N,NGu#*&_z6l');
define('LOGGED_IN_SALT',   '+Rp0!2{-zvxAN0`WY4-zsmu2D%W.43!oKvlJ0lPf,R-gim:7h2WC:xWM95d*;ti<');
define('NONCE_SALT',       'KIQ{Ms^W}d&(BpL+o`>>eK2.v,i+9e4e(Vz._Dx>|u,I81hy0;Yb4I p r9/i{ms');

/**#@-*/

/**
 * WordPress Database Table prefix.
 *
 * You can have multiple installations in one database if you give each a unique
 * prefix. Only numbers, letters, and underscores please!
 */
$table_prefix  = 'wp_';

/**
 * WordPress Localized Language, defaults to English.
 *
 * Change this to localize WordPress.  A corresponding MO file for the chosen
 * language must be installed to wp-content/languages. For example, install
 * de.mo to wp-content/languages and set WPLANG to 'de' to enable German
 * language support.
 */
define ('WPLANG', '');

/**
 * For developers: WordPress debugging mode.
 *
 * Change this to true to enable the display of notices during development.
 * It is strongly recommended that plugin and theme developers use WP_DEBUG
 * in their development environments.
 */
define('WP_DEBUG', false);

/* That's all, stop editing! Happy blogging. */

/** Absolute path to the WordPress directory. */
if ( !defined('ABSPATH') )
	define('ABSPATH', dirname(__FILE__) . '/');

/** Sets up WordPress vars and included files. */
require_once(ABSPATH . 'wp-settings.php');

