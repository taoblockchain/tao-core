/*=========================================================================================
    File Name: account-profile.js
    Description: Bootstrap switch and switchery are best switches with many options.
    ----------------------------------------------------------------------------------------
    Item Name: Crypto ICO - Cryptocurrency Website Landing Page HTML + Dashboard Template
    Version: 1.0
    Author: PIXINVENT
    Author URL: http://www.themeforest.net/user/pixinvent
==========================================================================================*/
Number.prototype.formatMoney = function(c, d, t){
    var n = this, 
    c = isNaN(c = Math.abs(c)) ? 2 : c, 
    d = d == undefined ? "." : d, 
    t = t == undefined ? "," : t, 
    s = n < 0 ? "-" : "", 
    i = String(parseInt(n = Math.abs(Number(n) || 0).toFixed(c))), 
    j = (j = i.length) > 3 ? j % 3 : 0;
   return s + (j ? i.substr(0, j) + t : "") + i.substr(j).replace(/(\d{3})(?=\d)/g, "$1" + t) + (c ? d + Math.abs(n - i).toFixed(c).slice(2) : "");
 };

(function(window, document, $) {
  'use strict';

  // var switchery = new Switchery('.switchery', { size: 'small' });
  var $html = $('html');
    
    // Switchery
    var i = 0;
    if (Array.prototype.forEach) {

        var elems = $('.switchery');
        $.each( elems, function( key, value ) {
            var $size="", $color="",$sizeClass="", $colorCode="";
            $size = $(this).data('size');
            var $sizes ={
                'lg' : "large",
                'sm' : "small",
                'xs' : "xsmall"
            };
            if($(this).data('size')!== undefined){
                $sizeClass = "switchery switchery-"+$sizes[$size];
            }
            else{
                $sizeClass = "switchery";
            }

            $color = $(this).data('color');
            var $colors ={
                'primary' : "#967ADC",
                'success' : "#37BC9B",
                'danger' : "#DA4453",
                'warning' : "#F6BB42",
                'info' : "#3BAFDA"
            };
            if($color !== undefined){
                $colorCode = $colors[$color];
            }
            else{
                $colorCode = "#37BC9B";
            }

            var switchery = new Switchery($(this)[0], { className: $sizeClass, color: $colorCode });
        });
    } else {
        var elems1 = document.querySelectorAll('.switchery');

        for (i = 0; i < elems1.length; i++) {
            var $size = elems1[i].data('size');
            var $color = elems1[i].data('color');
            var switchery = new Switchery(elems1[i], { color: '#37BC9B' });
        }
    }
    /*  Toggle Ends   */
     var worker1 = new Worker('/app-assets/js/scripts/forms/ww_priceticker.js');     
     worker1.onmessage = function (event) {
        var obj = JSON.parse(event.data);
        if ($('[id="price_type"]').is(':checked')) {   // USD Price
            var m = $('#current_price');
            m.text("$" + obj.data.quotes.USD.price.formatMoney(8, '.', ','));
            $('[name="currency_type"]').text('');
        } else {                                // BTC Price
            var m = $('#current_price');
            $('[name="currency_type"]').text('BTC');
            m.text(obj.data.quotes.BTC.price.formatMoney(8, '.', ','));
        }
     };
     var worker2 = new Worker('/app-assets/js/scripts/forms/ww_getinfo.js');     
     worker2.onmessage = function (event) {
        var obj = JSON.parse(JSON.parse(event.data));
        $('#current_xto').text(obj.balance);
        var val = obj.balance * $('#current_price').text().replace('$','');
        if ($('[id="price_type"]').is(':checked')) {   // USD Price
            var cur_val = "$" + val.formatMoney(2, '.', ',');
        } else {
            var cur_val = val.formatMoney(8, '.', ',');
        }
        $('#total_value').text(cur_val);
        $('[name=last_block]').text(obj.blocks);
        $('[name=connections]').text(obj.connections);
        $('[id=stake_xto]').text(obj.stake);
        $('[id=total_xto]').html("<h4>"+ (obj.stake + obj.balance )+"</h4>");
     };
     var worker3 = new Worker('/app-assets/js/scripts/forms/ww_btcprice.js');     
     worker3.onmessage = function (event) {
        var obj = JSON.parse(event.data);
        $('[name="btc_price"]').text("Last Bitcoin Price: $" + obj.data.quotes.USD.price.formatMoney(2, '.', ','));
     };
 })(window, document, jQuery);