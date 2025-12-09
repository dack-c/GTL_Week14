function DoClear()  
  GlobalConfig.bIsGameClear = true
  print("Game Clear")
end

function DoDeath()
  GlobalConfig.bIsPlayerDeath = true
  SetSlomo(1.5, 0.2)  -- 1.5초 동안 20% 속도로 슬로모션 (5배 느려짐)
  print("Player Death")
end
