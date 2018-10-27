var sass = require('gulp-sass');
var merge = require('merge-stream');
var autoprefixer = require('gulp-autoprefixer');

module.exports = function(gulp, callback) {
	var custom = gulp.src(config.source.sass + '/custom-rtl.scss')
		.pipe(sass().on('error', sass.logError))
		.pipe(autoprefixer({
            browsers: config.autoprefixerBrowsers,
            cascade: false
        }))
		.pipe(gulp.dest(config.destination.css_rtl));

	var style = gulp.src(config.assets + '/scss/style-rtl.scss')
		.pipe(sass().on('error', sass.logError))
		.pipe(autoprefixer({
            browsers: config.autoprefixerBrowsers,
            cascade: false
        }))
		.pipe(gulp.dest(config.assets + '/css/'));

	return merge(custom, style);
};