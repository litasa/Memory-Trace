

binToHex = function(binArray) {
  var out = new String();
  out = '0x';
  for(var i = 0; i < binArray.length ; ++i){
    if(binArray[i] < 16){
      out += '0';
    }
    out += binArray[i].toString(16);
  }
  return out;
}

intToHex = function(int) {
  var out = Number(int).toString(16);
  if((out.length % 2) != 0) {
    out = '0' + out;
  }
  return out;
}

hexToInt = function(hexString) {
  return parseInt(hexString, 16)
}

binToInt = function(bin) {
  return hexToInt(binToHex(bin));
}

hexToBin = function(hexString) {
  var out = new Array();
  var string;
  if(hexString.slice(0,2) == '0x')
  {
    string = hexString.slice(2); //remove 0x
  }
  else {
    string = hexString;
  }
  for(var i = 0; i < string.length; ++i){
    var hex = string.charAt(i) + string.charAt(++i);
    out.push(parseInt(hex,16));
  }
  return out;
}

removeLeadingZeroes = function(hexString) {
  var string = "";
  if(hexString.slice(0,2) == '0x')
  {
    string = hexString.slice(2); //remove 0x
  }
  else {
    string = hexString;
  }
  var i = 0;
  while (string.charAt(i) == '0') {
    string = string.slice(1);
    ++i;
  }
  return string;
}

addIntToHex = function(hexString,size) {
  var bin = hexToBin(hexString);
  var binSize = hexToBin(intToHex(size));
  var out = bin;
  var minLength = Math.min(bin.length, binSize.length);
  var carry = 0;
  for(var i = 1; i <= minLength; ++i) {
    var num = bin[bin.length - i] + binSize[binSize.length - i] + carry;
    if(num > 255){
      carry = 1;
    }
    else {
      carry = 0;
    }
    out[bin.length - i] = num;
  }
  if(carry){
    throw "overflow"
  }
  return out;
}

addHexToHex = function(hexString1, hexString2) {
  var bin1 = hexToBin(hexString1);
  var bin2 = hexToBin(hexString2);
  var out = addBinToBin(bin1,bin2);
  return binToHex(out);
}

addBinToBin = function(bin1, bin2) {
  var out = bin1.slice(0); //copy by value
  var minLength = Math.min(bin1.length, bin2.length);
  var carry = 0;
  for(var i = 1; i <= minLength; ++i) {
    var num = bin1[bin1.length - i] + bin2[bin2.length - i] + carry;
    if(num > 255){
      carry = 1;
    }
    else {
      carry = 0;
    }
    out[bin1.length - i] = num;
  }
  if(carry){
    throw "overflow"
  }
  return out;
}

hexLess = function(hexString1, hexString2) {
  var hex1 = removeLeadingZeroes(hexString1);
  var hex2 = removeLeadingZeroes(hexString2);
  if(hex1.length < hex2.length) {
    return true;
  }
  else if (hex1.length == hex2.length) {

    for(var i = 0; i < hex1.length; ++i) {
      if(parseInt(hex1[i],16) < parseInt(hex2[i],16)) {
        return true;
      }
    }
    return false;
  }
  else {
    return false;
  }
}

binComp = function(bin1,bin2) {
  if (bin1.length === bin2.length) {
    for(var i =0; i < bin1.length;++i) {
      if(bin1[i] < bin2[i]) {
        return 'less';
      }
      else if (bin1[i] > bin2[i]) {
        return 'greater';
      }
    }
    return 'equal';
  }
  else if (bin1.length < bin2.length) {
    return 'less'
  }
  else {
    return 'greater';
  }
}

hexEquals = function(hexString1, hexString2) {
  var hex1 = removeLeadingZeroes(hexString1);
  var hex2 = removeLeadingZeroes(hexString2);
  if (hex1.length != hex2.length) {
    return false;
  }
  for(var i = 0; i < hex1.length; ++i) {
    if(parseInt(hex1[i],16) != parseInt(hex2[i],16)) {
      return false;
    }
  }
  return true;
}

binEquals = function(lhs,rhs) {
  if (lhs.length != rhs.length) {
    return false;
  }
  for(var i = 0; i < rhs.length; ++i) {
    if(lhs[i] != rhs[i]) {
      return false;
    }
  }
  return true;
}

hexGreater = function(hexString1, hexString2) {
  return !hexLess(hexString1,hexString2) && !hexEquals(hexString1,hexString2);
}

binGreater = function(lhs, rhs) {
  return !binLess(lhs,rhs) && !binEquals(lhs,rhs);
}

hexCompare = function(lhs, comp, rhs) {
  switch (comp) {
    case 'less':
      return hexLess(lhs,rhs);
      break;
    case 'greater':
      return hexGreater(lhs,rhs);
      break;
    case 'equal':
      return hexEquals(lhs,rhs);
      break;
    case 'less or equal':
      return hexLess(lhs,rhs) || hexEquals(lhs,rhs);
      break;
    case 'greater or equal':
      return hexGreater(lhs,rhs) || hexEquals(lsh, rhs);
      break;
    default:
      throw "Unknown compare";
  }
}

binCompare = function(lhs, comp, rhs) {
  var ret = binComp(lhs,rhs);

  switch (comp){
    case 'less':
    case '<':
      return ret == 'less';
      break;
    case 'greater':
    case '>':
      return ret == 'greater';
      break;
    case 'equal':
    case '==':
      return ret == 'equal';
      break;
    case 'less or equal':
    case '<=':
      return ret == 'less' || ret == 'equal';
      break;
    case 'greater or equal':
    case '>=':
      return ret == 'greater' || ret == 'equal';
      break;
    default:
      throw "Unknown compare";
  }
}
