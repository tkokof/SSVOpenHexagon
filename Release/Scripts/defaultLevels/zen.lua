-- include useful files
execScript("utils.lua")
execScript("common.lua")
execScript("commonpatterns.lua")

-- this function adds a pattern to the timeline based on a key
function addPattern(mKey)
	if mKey == 0 then pBarrageSpiral(math.random(5, 9), 0.41, 1)
	elseif mKey == 1 then pMirrorSpiralDouble(math.random(8, 10), 0)
	elseif mKey == 2 then pMirrorSpiral(math.random(2, 5), 0)
	end
end

-- shuffle the keys, and then call them to add all the patterns
-- shuffling is better than randomizing - it guarantees all the patterns will be called
keys = { 0, 0, 1, 1, 2 }
keys = shuffle(keys)

for i = 0, table.getn(keys) do
	addPattern(keys[i])
end

-- end - the file will restart from the beginning when the timeline is clear