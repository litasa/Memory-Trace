function pointerInsideCore(pointer, core)
{
  var less = hexCompare(pointer, 'less', core.start);
  var great = hexCompare(pointer, 'greater', core.end);
  return !less && !great;
}
