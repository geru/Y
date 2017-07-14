<?php
/**
 * Created by: hugh
 * Date: 7/6/17
 * Time: 8:53 AM
 */
/** YAML parsing callbacks *********************************/

/**
 * YAML parse callback general function call
 * @return mixed|null
 */
function y__yaml_callback_call_user_func_array( $value, $tag, $flags ) {
  if( is_array($value) ) {
    $function = array_shift($value);
  } else {
    $function =  $value;
    $value = array();
  }
  return function_exists($function) ? call_user_func_array($function, $value) : NULL;
}

/**
 * YAML parse callback Drupal t() translate
 * @return mixed|null
 */
function y__yaml_callback_t( $value, $tag, $flags ) {
  return t($value);
}

function y__yaml_callback_plugin_module( $value, $tag, $flags ) {
  // $plugin = & drupal_static('y__yaml_plugin_values', array());
  $params = _y__stack_peek('y__yaml_plugin_values');
  return $params['module'];
}

function y__yaml_callback_plugin_path( $value=NULL, $tag, $flags ) {
  // $plugin = & drupal_static('y__yaml_plugin_values', array());
  $params = _y__stack_peek('y__yaml_plugin_values');
  if( $value === NULL )
    return rtrim($params['path'], '/');
  return $params['path'] . $value;
}

/*
 * return the path relative to the module directory
 */
function y__yaml_callback_plugin_relativepath( $value=NULL, $tag, $flags ) {
  $path = '';
  // $plugin = & drupal_static('y__yaml_plugin_values', array());
  $params = _y__stack_peek('y__yaml_plugin_values');
  $module_path = drupal_get_path('module', $params['module']);
  if( ($pos = strrpos($params['path'], $module_path)) !== FALSE ) {
    $path = substr($params['path'], $pos + strlen($module_path) + 1);
    if ($value === NULL) {
      $path = rtrim($path, '/');
    } else {
      $path = $path . $value;
    }
  }
  return $path;
}

function y__yaml_callback_plugin_name( $value, $tag, $flags ) {
  // $plugin = & drupal_static('y__yaml_plugin_values', array());
  $params = _y__stack_peek('y__yaml_plugin_values');
  if( $value === NULL ) return $params['name'];
  return $params['path'] . $params['name'] . $value;
}

/*
 * YAML callback to return local variables.
 * If a single value is sent, then it is the local variable to return.
 * If an array of values is sent, then an array of those values are returned.
 * If an array of key/pair is sent then local values[key] are returned as returned values[pair]
 */
function y__yaml_callback_plugin_local( $values, $tag, $flags ) {
  $params = _y__stack_peek('y__yaml_plugin_values');
  if( !is_array($values) ) {
    return isset($params[$values]) ? $params[$values] : '';
  }
  $ret = array();
  foreach( $values as $key => $value ) {
    if( is_integer($key) ) {
      if( isset($params[$value]) ) {
        $ret[] = $params[$value];
      }
    } else {
      if( isset($params[$key]) ) {
        $ret[$value] = $params[$key];
      }
    }
  }
  return $ret;
}

function y__yaml_callback_plugin_modulepath( $value, $tag, $flags ) {
  // $plugin = & drupal_static('y__yaml_plugin_values', array());
  $params = _y__stack_peek('y__yaml_plugin_values');
  $module = $params['module'];
  $modulepath = drupal_get_path('module', $module);
  if( $value ){
    $modulepath .= '/' . $value;
  }
  return $modulepath;
}

function y__yaml_callback_eval( $value, $tag, $flags ) {
  $ret = array();
  if( !is_array($value) ) {
    $value = array( $value );
  }
  foreach( $value as $key => $evalline ) {
    $evalline = '$ret[$key]=' . $evalline . ';';
    eval($evalline);
  }
  $retval = ((count($ret) == 1) && is_numeric($key)) ? reset($ret) : $ret;
  return $retval;
}

/**
 * YAML parse callback general variable functionality
 * @return mixed|null
 */
function y__yaml_callback_var( $value, $tag, $flags ) {
  static $idx = 0;
  $staticvar = & drupal_static( 'y__static ' . $tag, array() );

  if( !is_array($value) ) {
    $staticvar[$idx] = $value;
  } else {
    foreach( $value as $opcode => $target ) {
      if( is_numeric($opcode) ) {
        $translate = array( '++' => 'inc', 'inc' => 'inc',
          '--' => 'dec', 'dec' => 'dec');
        $opcode = $translate[$target] ?? NULL;
        $target = $opcode ? 0 : 1;
      }
      switch( $opcode ) {
        case 'inc':
        case '+':
          $staticvar[$idx] = $staticvar[$idx] + $target;
          break;
        case 'dec':
        case '-':
          $staticvar[$idx] = $staticvar[$idx] - $target;
          break;
        default:
          $staticvar[$opcode] = $target;
      }
    }
  }
  return $staticvar;
}

// push var onto top of the stack
function _y__stack_push( $stackname, $var ) {
  $stack = & drupal_static($stackname, array());
  array_push($stack, $var);
}

// pop top element off stack
function _y__stack_pop( $stackname ) {
  $stack = & drupal_static($stackname, array());
  return array_pop($stack);
}

// get value of current stop of stack
function _y__stack_peek( $stackname ) {
  $stack = & drupal_static($stackname, array());
  return end($stack);
}

// change current element at top of stack (equiv of pop then push
function _y__stack_set( $stackname, $var ) {
  $stack = & drupal_static($stackname, array());
  array_pop($stack);
  array_push($stack, $var);
}

/**
 * YAML parse callback Set a plugin value
 * @return mixed|null
 */
function y__yaml_callback_plugin_set( $value, $tag, $flags ) {
  //$plugin = & drupal_static('y__yaml_plugin_values', array());
  $plugin = & _y__stack_peek('y__yaml_plugin_values');
  $plugin = array_merge( $plugin, $value );
  return $value;
}

/**
 * YAML parse callback set info for menu API discoverable plugin
 * @return mixed|null
 * */
function y__yaml_callback_plugin_menucallback( $value, $tag, $flags ) {
  $plugin = _y__stack_peek('y__yaml_plugin_values');
  $includefilearray = array(  'file' => $plugin['file'],
    'file path' => trim($plugin['path'], "/\\") );
  if( !$value || !is_array($value) ) {
    $ret = $includefilearray;
  } else {
    $ret = $value + $includefilearray;
  }
  return $ret;
}

function y__yaml_callback_plugin_callback( $value, $tag, $flags ) {
  $result = array();
  $includefilearray = array();
  // $plugin = & drupal_static('y__yaml_plugin_values', array());
  $plugin = _y__stack_peek('y__yaml_plugin_values');
  if( !is_array($value)) {
    $result['function'] = $value;
    $includefilearray['uri'] = $plugin['uri'];
  } else {
    if( !empty($value['plugin']) ) {
      $keys = is_array($value['plugin']) ? $value['plugin'] : array($value['plugin']);
      foreach( $keys as $key ) {
        $includefilearray[$key] = $plugin[$key];
      }
      unset( $value['plugin'] );
    }
    $result = $value;
  }
  $result += $includefilearray;
  return $result;
}

function y__yaml_callback_plugin_protocol( $value, $tag, $flags ) {
  // $plugin = & drupal_static('y__yaml_plugin_values', array());
  $plugin = _y__stack_peek('y__yaml_plugin_values');
  $params = $value + $plugin;
  $yinfo = y__protocol( $params['target'], $params['api'], $params['module'] );
  $ret = $yinfo[$params['id']] ?? array();
  return $ret;
}

/**
 * Set plugin values
 * @param array $initarray

function y__yaml_plugin_values( $initarray=array() ) {
$plugin = & drupal_static('y__yaml_plugin_values', array());
$plugin = $initarray;
}
 */
