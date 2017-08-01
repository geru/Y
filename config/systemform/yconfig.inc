<?php

/**
 * Quick n dirty form to turn Y cacheing on and off
 * @param $form
 * @param $form_state
 * @return mixed
 */
function y__config_form($form, &$form_state) {
  $yinfo = y__protocol('systemform', 'form', 'y');
  foreach( $yinfo['yconfig'] as $name => $forminfo ) {
    if( $forminfo['#type'] != 'item' )
    $forminfo['#default_value'] = variable_get($name, TRUE);
    $form[$name] = $forminfo;
  }
  return system_settings_form($form);
}

