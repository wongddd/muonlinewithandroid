-- Agregar la columna HolyInventory a la tabla Character
ALTER TABLE Character 
ADD HolyInventory VARBINARY(252);

GO
-- Actualizar todos los registros para establecer HolyInventory con 0x0 repetido 252 veces
UPDATE Character 
SET HolyInventory = CONVERT(VARBINARY(252), REPLICATE(0x0, 252));

GO

-- Agregar la columna HolyInventory a la tabla DefaultClassType
ALTER TABLE DefaultClassType 
ADD HolyInventory VARBINARY(252);

GO
-- Actualizar todos los registros para establecer HolyInventory con 0x0 repetido 252 veces
UPDATE DefaultClassType 
SET HolyInventory = CONVERT(VARBINARY(252), REPLICATE(0x0, 252));
