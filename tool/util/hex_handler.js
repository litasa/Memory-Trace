compareBuffers = function(lhs,rhs) {
  if(lhs.length < rhs.length) {
    return 'less';
  }
  else if(lhs.length > rhs.length) {
    return 'greater';
  }
  for(var i = 0; i < lhs.length; ++i) {
    if(lhs[i] < rhs[i]) {
      return 'less';
    }
    else if( lhs[i] > rhs[i]) {
      return 'greater';
    }
  }
  return 'equal';
}

bufferToInt = function(buff) {
  
}