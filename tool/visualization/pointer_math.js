function pointerInsideCore(pointer, core)
{
  if(typeof pointer === "string") {
    var less = hexCompare(pointer, 'less', core.start);
    var great = hexCompare(pointer, 'greater', core.end);
  }
  else {
    var less = binCompare(pointer, 'less', core.start);
    var great = binCompare(pointer, 'greater', core.end);
  }

  return !less && !great;
}
